#include "AnimatedLpgProcessor.h"
#include "AnimatedLpgEditor.h"


namespace AnimatedLpg
{
    //==============================================================================
    AnimatedLpgProcessorEditor::AnimatedLpgProcessorEditor (AnimatedLpgAudioProcessor& p)
        : AudioProcessorEditor (&p), processorRef (p)
    {
        constexpr int initWidth = 800;
        constexpr int initHeight = 450;
        setSize(initWidth, initHeight);
        setWantsKeyboardFocus(false);
        webGpuWindow.initialize();
        processorRef.setResponseCallback([this](const std::string& result) {
        juce::MessageManager::callAsync([this, result]() {
          setWantsKeyboardFocus(false);
        });
    });
}

    AnimatedLpgProcessorEditor::~AnimatedLpgProcessorEditor()
    {
        processorRef.setResponseCallback(nullptr);
        stopTimer();
        processorRef.savedCameraState = webGpuWindow.getScene().getCameraState();
        webGpuWindow.terminate();
    }

   void AnimatedLpgProcessorEditor::parentHierarchyChanged()
{
    AudioProcessorEditor::parentHierarchyChanged();

    if (!webGpuWindow.hasSurface() && getPeer() != nullptr) {
        const auto* primaryDisplay = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
        const double scale = primaryDisplay ? primaryDisplay->scale : 1.0;
        const auto width  = static_cast<uint32_t>(std::round(static_cast<double>(getWidth())  * scale));
        const auto height = static_cast<uint32_t>(std::round(static_cast<double>(getHeight()) * scale));

        if (!webGpuWindow.initSurface(scale, width, height))
            return;

       #if JUCE_MAC
        // The Metal layer lives in its own NSView, confined to this editor, so
        // it never paints over the standalone window's title bar.
        // The native view's -hitTest: returns nil so AppKit hands mouse events to
        // JUCE's peer; this makes JUCE's own routing skip the overlay component too,
        // so mouseDown reaches the editor (slider/text hit-testing) instead of dying
        // in the NSViewComponent. (mouseWheel already bubbles to the parent.)
        mMetalView.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(mMetalView);
        mMetalView.setView(webGpuWindow.getNativeView());
        mMetalView.setBounds(getLocalBounds());
       #endif

        mStartTimeMs = juce::Time::getMillisecondCounterHiRes();
        mStartTimeSet = true;
        mConfiguredW = width;
        mConfiguredH = height;
        webGpuWindow.getScene().setCameraState(processorRef.savedCameraState);
        startTimerHz(60);
        juce::MessageManager::callAsync([this]() {
            setResizable(true, true);
            setResizeLimits(200,
                            113,
                            3200,
                            1800);
            getConstrainer()->setFixedAspectRatio(800.0 / 450.0);
        });
    }
}

void AnimatedLpgProcessorEditor::timerCallback()
{

    if (mResizePending) {
        if (mPendingW != mConfiguredW || mPendingH != mConfiguredH) {
            webGpuWindow.onResize(mPendingW, mPendingH);
            mConfiguredW = mPendingW;
            mConfiguredH = mPendingH;
        }
        mResizePending = false;
    }

    const double now = juce::Time::getMillisecondCounterHiRes();

    if (!mStartTimeSet) {
        mStartTimeMs   = now;
        mLastFrameMs   = now;
        mStartTimeSet  = true;
    }

    double dt = (now - mLastFrameMs) * 0.001;
    mLastFrameMs = now;

    // Clamp: a stall (model warmup, resize, etc.) can't pop the sim
    dt = juce::jlimit(0.0, 1.0 / 30.0, dt);

    mElapsed += dt;  // accumulate clamped time

    webGpuWindow.getScene().renderFrame(static_cast<float>(mElapsed));
}

void AnimatedLpgProcessorEditor::setResizeReady()
{
    setResizable(true, true);
}

//==============================================================================
void AnimatedLpgProcessorEditor::resized()
{
    if (!webGpuWindow.hasSurface()) return;
   #if JUCE_MAC
    mMetalView.setBounds(getLocalBounds());
   #endif
    const auto* primaryDisplay = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    const float scale = primaryDisplay ? static_cast<float>(primaryDisplay->scale) : 1.0f;
    mPendingW = static_cast<uint32_t>(std::round(static_cast<float>(getWidth())  * scale));
    mPendingH = static_cast<uint32_t>(std::round(static_cast<float>(getHeight()) * scale));
    mResizePending = true;
}

//==============================================================================
void AnimatedLpgProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
        juce::ignoreUnused(event);
}

void AnimatedLpgProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
        if (mCameraDragging)
        {
            const auto x = static_cast<float>(event.x);
            const auto y = static_cast<float>(event.y);
            webGpuWindow.getScene().onMouseMove(x, y);
            mLastMouseX = x;
            mLastMouseY = y;
        }
}

void AnimatedLpgProcessorEditor::mouseUp(const juce::MouseEvent& e)
{

    if (mCameraDragging)
    {
        webGpuWindow.getScene().onMouseButton(0, true,
                                      static_cast<float>(e.x),
                                      static_cast<float>(e.y));
        mCameraDragging = false;
    }
}

void AnimatedLpgProcessorEditor::mouseWheelMove(const juce::MouseEvent& e,
                                                      const juce::MouseWheelDetails& wheel)
{
    juce::ignoreUnused(e);
    webGpuWindow.getScene().onScroll(wheel.deltaX, wheel.deltaY);
}


bool AnimatedLpgProcessorEditor::keyPressed(const juce::KeyPress& key)
{

    if (key == juce::KeyPress::escapeKey)
    {
        setWantsKeyboardFocus(false);
        return true;
    }

    if (key == juce::KeyPress::backspaceKey)
    {
        if (!mTypedText.empty())
            mTypedText.pop_back();
    }
    else if (key.getTextCharacter() >= ' ')
    {
        mTypedText += static_cast<char>(key.getTextCharacter());
    }
    else
    {
        return false;
    }

    return true;
}
}

