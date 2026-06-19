#pragma once
#include "AnimatedLpgProcessor.h"

namespace AnimatedLpg
{
    //==============================================================================
    class AnimatedLpgProcessorEditor final : public juce::AudioProcessorEditor
    {
    public:
        explicit AnimatedLpgProcessorEditor (AnimatedLpgAudioProcessor&);
        ~AnimatedLpgProcessorEditor() override;

        //==============================================================================
        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        // This reference is provided as a quick way for your editor to
        // access the processor object that created it.
        AnimatedLpgAudioProcessor& processorRef;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedLpgProcessorEditor)
    };
}


