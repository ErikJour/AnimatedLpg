//
// Created by Erik Jourgensen on 6/20/26.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace AnimatedLPG
{
    class AnimatedLPGEffect
    {
    public:
        AnimatedLPGEffect();
        ~AnimatedLPGEffect();
        void reset(double sampleRate);
        static void render(float** outputBuffers, int sampleCount);
    private:
        double mSampleRate;
    };

}




