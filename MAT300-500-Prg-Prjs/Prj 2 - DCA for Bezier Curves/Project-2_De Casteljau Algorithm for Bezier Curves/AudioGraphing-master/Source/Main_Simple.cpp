/*
  ==============================================================================

    Main.cpp
    MATH 500 - Project 2

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainContentComponent_new.h"

class BezierApplication : public juce::JUCEApplication
{
public:
    BezierApplication() = default;

    const juce::String getApplicationName() override { return "MATH 500 - Project 2: Bezier Curves"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName(), new MainContentComponent(), *this));
    }

    void shutdown() override { mainWindow = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, juce::Component* c, JUCEApplication& a)
            : DocumentWindow(name, 
                           juce::Desktop::getInstance().getDefaultLookAndFeel()
                               .findColour(juce::ResizableWindow::backgroundColourId),
                           juce::DocumentWindow::allButtons),
              app(a)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);

#if JUCE_ANDROID || JUCE_IOS
            setFullScreen(true);
#else
            setResizable(true, false);
            setResizeLimits(800, 600, 10000, 10000);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication& app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(BezierApplication)
