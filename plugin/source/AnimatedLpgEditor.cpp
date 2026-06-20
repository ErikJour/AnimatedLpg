#include "AnimatedLpgProcessor.h"
#include "AnimatedLpgEditor.h"

namespace AnimatedLpg
{
    //==============================================================================
    AnimatedLpgProcessorEditor::AnimatedLpgProcessorEditor (AnimatedLpgAudioProcessor& p)
        : AudioProcessorEditor (&p), processorRef (p)
    {
        juce::ignoreUnused (processorRef);
        // Make sure that before the constructor has finished, you've set the
        // editor's size to whatever you need it to be.
        setSize (400, 300);
    }

    AnimatedLpgProcessorEditor::~AnimatedLpgProcessorEditor() = default;

    //==============================================================================
    void AnimatedLpgProcessorEditor::paint (juce::Graphics& g)
    {
       juce::ignoreUnused (g);
    }

    void AnimatedLpgProcessorEditor::resized()
    {

    }
}

