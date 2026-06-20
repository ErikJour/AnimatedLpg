//
// Created by Erik Jourgensen on 6/20/26.
//

#include "AnimatedLPGEffect.h"

namespace AnimatedLPG
{

    AnimatedLPGEffect::AnimatedLPGEffect() : mSampleRate(0)
    {
    }

    AnimatedLPGEffect::~AnimatedLPGEffect() {}
    void AnimatedLPGEffect::reset(const double sampleRate)
    {
        mSampleRate = sampleRate;
    }
    void AnimatedLPGEffect::render(float** outputBuffers, int sampleCount)
    {
        float* outputBufferLeft = outputBuffers[0];
        float* outputBufferRight = outputBuffers[1];

        for (int i = 0; i < sampleCount; i++)
        {
            //process buffer
        }
    }

}