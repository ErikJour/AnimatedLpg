#pragma once
#include "AnimatedLpgProcessor.h"
#if JUCE_MAC
 #include <juce_gui_extra/juce_gui_extra.h>
#endif
#include "WebGpuWindow.h"
#include "scene.h"

namespace AnimatedLpg
{
    //==============================================================================
    class AnimatedLpgProcessorEditor : public juce::AudioProcessorEditor,
                                                    private juce::Timer
    {
    public:
        explicit AnimatedLpgProcessorEditor (AnimatedLpgAudioProcessor&);
        ~AnimatedLpgProcessorEditor() override;
        void mouseDown   (const juce::MouseEvent& e) override;
        void mouseDrag   (const juce::MouseEvent& e) override;
        void mouseUp     (const juce::MouseEvent& e) override;
        bool keyPressed  (const juce::KeyPress& key) override;

        //==============================================================================
        void resized() override;

    private:
        void parentHierarchyChanged() override;
        void timerCallback() override;
        void setResizeReady();
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
        AnimatedLpgAudioProcessor& processorRef;
        WebGpuWindow webGpuWindow;
#if JUCE_MAC
        juce::NSViewComponent mMetalView;
#endif
        std::string mTypedText;
        bool     mDragging      = false;
        float    mDragOffset    = 0.0f;
        int      mActiveSlider  = 0;

        bool     timerReady     = false;
        bool     mResizePending = false;
        uint32_t mPendingW      = 0;
        uint32_t mPendingH      = 0;
        uint32_t mConfiguredW   = 0;
        uint32_t mConfiguredH   = 0;

        //Camera
        bool  mCameraDragging = false;
        float mLastMouseX     = 0.0f;
        float mLastMouseY     = 0.0f;

        double mStartTimeMs = 0.0;
        double mLastFrameMs = 0.0;
        double mElapsed     = 0.0;
        bool   mStartTimeSet = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedLpgProcessorEditor)
    };
}


