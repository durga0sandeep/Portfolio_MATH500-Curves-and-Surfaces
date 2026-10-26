/*
  ==============================================================================
    Main.cpp
    MATH 500 - Project 3: 
  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainContentComponent_new.h"  // Must define class MainContentComponent

class MathApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "MATH 500 - Project 3: Interpolating Polynomials"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName(), *this);
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, juce::JUCEApplication& appRef)
            : juce::DocumentWindow(name,
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                juce::DocumentWindow::allButtons),
            app(appRef)
        {
            setUsingNativeTitleBar(true);

            // This is the ONE page/component your app shows.
            setContentOwned(new MainContentComponent(), true);

            setResizable(true, true);
            setResizeLimits(800, 600, 4096, 4096);
            centreWithSize(1000, 700);

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        juce::JUCEApplication& app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(MathApplication)