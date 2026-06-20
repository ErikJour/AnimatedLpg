//
// Created by Erik Jourgensen on 6/4/26.
//
#pragma once
#include <algorithm>
#include <juce_audio_processors/juce_audio_processors.h>

//Low-pass gate based on this paper:
//https://www.researchgate.net/publication/271835899_A_Digital_Model_of_the_Buchla_Lowpass-Gate
//Used Claude for implementation

namespace AnimatedLPG
{
    class AnimatedLPG
    {
    public:
        AnimatedLPG();
        ~AnimatedLPG();
        enum class Mode { Both, VCA, LowPass};

        void prepare(double sampleRate);
        void reset();
        void setMode(Mode mode);
        void setRf(float resistance);
        void setResonance(float newResonance);
        void updateResonance();
        [[nodiscard]] float getAmax() const;
        [[nodiscard]] float getRf() const;
        void processBuffer(float* buffer, int numSamples);

        template <typename RfSource>
            void processBufferModulated(float* buffer, int numSamples, RfSource&& nextRf);
        auto processSample(float yi,
                            float a1,
                            float a2,
                            float b1,
                            float b2,
                            float b3,
                            float b4,
                            float d1,
                            float d2,
                            float h,
                            float g,
                            float D,
                            float Q,
                            float denomYx) -> float;

    private:
        float C1 = 1e-9f;
        float C2 = 220e-12f;
        float C3 = 4.7e-9f;
        float Ra = 5e6f;
        float Rf = 100e3f;
        float a = 0.0f;

        double mSampleRate = { 44100.0 };
        float sVx = 0.0f;
        float sVout = 0.0f;
        float sDiff = 0.0f;
        Mode mMode = Mode::LowPass;

        juce::SmoothedValue<float> resonanceSmoothed;

        static constexpr float maxResonance = 0.95f;
    };
}


