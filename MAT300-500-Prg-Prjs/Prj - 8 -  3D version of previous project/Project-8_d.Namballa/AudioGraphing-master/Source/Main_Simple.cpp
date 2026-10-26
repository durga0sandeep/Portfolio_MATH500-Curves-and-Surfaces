#include <JuceHeader.h>
#include "MainContentComponent_new.h"

class BezierApplication : public juce::JUCEApplication
{
public:
    BezierApplication() = default;

    const juce::String getApplicationName() override
    {
        return "MATH 500 - Project 8: 3D Bezier Curves";
    }

    const juce::String getApplicationVersion() override
    {
        return "1.0.0";
    }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName(), new MainContentComponent(), *this));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, juce::Component* contentComponent, JUCEApplication& appRef)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(juce::ResizableWindow::backgroundColourId),
                             juce::DocumentWindow::allButtons),
              app(appRef)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(contentComponent, true);
            setResizable(true, false);
            setResizeLimits(1000, 720, 1800, 1200);
            centreWithSize(1280, 860);
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

START_JUCE_APPLICATION(BezierApplication)
