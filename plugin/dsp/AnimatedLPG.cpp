//
// Created by Erik Jourgensen on 6/20/26.
//
#include "AnimatedLPG.h"

namespace AnimatedLPG
{

AnimatedLPG::AnimatedLPG() = default;
AnimatedLPG::~AnimatedLPG() = default;

void AnimatedLPG::prepare(const double sampleRate)
{
    mSampleRate = static_cast<float>(sampleRate);
    Rf = 1e3f;
    resonanceSmoothed.reset(sampleRate, 0.01f);
    resonanceSmoothed.setCurrentAndTargetValue(0.5f);
    reset();
}

void AnimatedLPG::reset()
{
    sVx = sVout = sDiff = 0.0f;
}

void AnimatedLPG::setMode(const Mode mode)
{
    mMode = mode;
    switch (mode)
    {
            case Mode::Both:     C1 = 1e-9f;    C2 = 1e-9f;    C3 = 0.0f;    Ra = 5e6f; break;
            case Mode::VCA:      C1 = 220e-12f; C2 = 220e-12f; C3 = 0.0f;    Ra = 5e3f; break;
            case Mode::LowPass:  C1 = 1e-9f;    C2 = 220e-12f; C3 = 4.7e-9f; Ra = 5e6f; break;
            default: ;
    }
}

void AnimatedLPG::setRf(const float resistance)
{
    Rf = resistance;
}

void AnimatedLPG::setResonance(const float newResonance)
{
    resonanceSmoothed.setTargetValue(newResonance);
}

void AnimatedLPG::updateResonance()
{
    float resonance = resonanceSmoothed.getNextValue();
    resonance = std::clamp(resonance, 0.0f, maxResonance);
    a = (C3 > 0.0f) ? resonance * getAmax() : 0.0f;
}

[[nodiscard]] float AnimatedLPG::getAmax() const
{
    if (C3 <= 0.0f) return 0.0f;
    return (2.0f * C1 * Ra + (C2 + C3) * (Ra + Rf)) / (C3 * Ra);
}

[[nodiscard]] float AnimatedLPG::getRf() const
{
    return Rf;
}

void AnimatedLPG::processBuffer(float* buffer, const int numSamples)
{
    updateResonance();
    const float a1 = 1.0f / (C1 * Rf);
    const float a2 = -(1.0f / C1) * (1.0f / Rf + 1.0f / Ra);
    const float b1 = 1.0f  / (C2 * Rf);
    const float b2 = -2.0f / (C2 * Rf);
    const float b3 = 1.0f  / (C2 * Rf);
    const float b4 = C3 / C2;
    const float d1 = a;
    constexpr float d2 = -1.0f;
    const float h = 1.0f / (2.0f * static_cast<float>(mSampleRate));
    const float g = 2.0f * static_cast<float>(mSampleRate);
    const float D = 1.0f - h * a2;
    const float P = 1.0f - h * b2 - h * b4 * g * d2;
    const float Q = h * b3 + h * b4 * g * d1;
    const float denomYx = P - Q * h * a1 / D;

    for (int i = 0; i < numSamples; i++)
    {
        buffer[i] = processSample(buffer[i], a1, a2, b1, b2, b3, b4, d1, d2, h, g, D, Q, denomYx);
    }
}

template <typename RfSource>
void AnimatedLPG::processBufferModulated(float* buffer, const int numSamples, RfSource&& nextRf)
{
    const float h = 1.0f / (2.0f * static_cast<float>(mSampleRate));
    const float g = 2.0f * static_cast<float>(mSampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        Rf = nextRf();
        updateResonance();
        const float a1 = 1.0f / (C1 * Rf);
        const float a2 = -(1.0f / C1) * (1.0f / Rf + 1.0f / Ra);
        const float b1 = 1.0f / (C2 * Rf);
        const float b2 = -2.0f / (C2 * Rf);
        const float b3 = 1.0f / (C2 * Rf);
        const float b4 = C3 / C2;
        const float d1 = a;
        constexpr float d2 = -1.0f;

        const float D = 1.0f - h * a2;
        const float P = 1.0f - h * b2 - h * b4 * g * d2;
        const float Q = h * b3 + h * b4 * g * d1;
        const float denomYx = P - Q * h * a1 / D;

            buffer[i] = processSample(buffer[i], a1, a2, b1, b2, b3, b4, d1, d2, h, g, D, Q, denomYx);
    }
}

float AnimatedLPG::processSample(const float yi,
                            const float a1,
                            const float a2,
                            const float b1,
                            const float b2,
                            const float b3,
                            const float b4,
                            const float d1,
                            const float d2,
                            const float h,
                            const float g,
                            const float D,
                            const float Q,
                            const float denomYx)
    {
        const float yx = (sVx + h * b1 * yi + Q * sVout / D + h * b4 * sDiff) / denomYx;
        const float yo = (sVout + h * a1 * yx) / D;

        const float ud = d1 * yo + d2 * yx;
        const float yd = g * ud + sDiff;
        const float ux = b1 * yi + b2 * yx + b3 * yo + b4 * yd;
        const float uo = a1 * yx + a2 * yo;

        sVx = yx + h * ux;
        sVout = yo + h * uo;
        sDiff = -yd - g * ud;

        return yo;
    }

}
