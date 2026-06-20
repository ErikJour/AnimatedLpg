//
// Created by Erik Jourgensen on 6/20/26.
//

#include "AnimatedVactrol.h"
#include <algorithm>
#include <cmath>

//Vactrol based on this paper:
//https://www.researchgate.net/publication/271835899_A_Digital_Model_of_the_Buchla_Lowpass-Gate
//Used Claude for implementation

namespace AnimatedLPG
{
    AnimatedVactrol::AnimatedVactrol() {}
AnimatedVactrol::~AnimatedVactrol() {}

void AnimatedVactrol::prepare(const double sampleRate)
{
    mSampleRate = static_cast<float>(sampleRate);
    updateCoefficients();
    reset();
}

void AnimatedVactrol::reset()
{
    // Start dim: tiny LED current = very high resistance (gate closed)
    mCurrent = kMinCurrent;
}

void AnimatedVactrol::setAttackTime(const float seconds)
{
    mAttackTime  = std::max(seconds, 1e-4f); updateCoefficients();
}
void AnimatedVactrol::setReleaseTime(const float seconds)
{
    mReleaseTime = std::max(seconds, 1e-4f); updateCoefficients();
}

void AnimatedVactrol::setControl(float norm)
{
    // Control input in [0, 1]; maps to target LED current.
    // 0 -> dark/closed, 1 -> full drive/open.
    norm = std::clamp(norm, 0.0f, 1.0f);
    mTargetCurrent = kMinCurrent + norm * (kMaxCurrent - kMinCurrent);
}

void AnimatedVactrol::strike(const float velocity)
{
    // Trigger a strike: velocity in [0, 1] sets how hard the LED is driven.
    // Self-contained pluck: the LED jumps bright instantly, then immediately
    // begins decaying back toward dark on the release time constant. The
    // roll-off is part of the strike, so it no longer depends on note length.
    const float norm = std::clamp(velocity, 0.0f, 1.0f);
    mCurrent       = kMinCurrent + norm * (kMaxCurrent - kMinCurrent);  // instant attack
    mTargetCurrent = kMinCurrent;                                       // roll off to dark
}

void AnimatedVactrol::releaseGate()
{
    // Begin decay back toward darkness (e.g. on note-off, or always for a pluck).
    mTargetCurrent = kMinCurrent;
}

float AnimatedVactrol::tick()
{
    // Advance one sample. Returns the current vactrol resistance in ohms.

    // Asymmetric one-pole: choose coefficient by direction of travel.
    const bool rising = (mTargetCurrent > mCurrent);
    float coeff = rising ? mAttackCoeff : mReleaseCoeff;
    // Datasheet note: the vactrol responds faster at higher light levels.
    // Apply this nudge only while brightening — on release we want the
    // full release time constant to shape the pluck, otherwise a strike
    // (which starts at full current) would slam shut in ~1 sample.
    if (rising)
    {
        const float levelNorm = std::clamp(
        (mCurrent - kMinCurrent) / (kMaxCurrent - kMinCurrent), 0.0f, 1.0f);
        coeff = coeff + (1.0f - coeff) * (0.5f * levelNorm);
    }

    mCurrent += coeff * (mTargetCurrent - mCurrent);
    mCurrent = std::clamp(mCurrent, kMinCurrent, kMaxCurrent);

    return currentToResistance(mCurrent);
}

float AnimatedVactrol::getRf() const
{
    return currentToResistance(mCurrent);
}

float AnimatedVactrol::currentToResistance(const float If)
{
    return kA / std::pow(If, 1.4f) + kB;   // Eq. 39
}

void AnimatedVactrol::updateCoefficients()
{
    // One-pole time-constant -> per-sample coefficient.
    mAttackCoeff  = 1.0f - std::exp(-1.0f / (mAttackTime  * mSampleRate));
    mReleaseCoeff = 1.0f - std::exp(-1.0f / (mReleaseTime * mSampleRate));
}
}



