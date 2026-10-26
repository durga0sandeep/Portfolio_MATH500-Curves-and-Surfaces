#pragma once

#include <JuceHeader.h>
#include "BezierComponent.h"

class MainContentComponent : public juce::Component
{
public:
    MainContentComponent();
    ~MainContentComponent() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BezierComponent bezierView;

    juce::TextButton methodNLIButton;
    juce::TextButton methodBBButton;
    juce::TextButton methodMSButton;

    juce::Slider subdivisionSlider;
    juce::Label subdivisionLabel;
    juce::Slider tValueSlider;
    juce::Label tValueLabel;

    juce::Slider yawSlider;
    juce::Label yawLabel;
    juce::Slider pitchSlider;
    juce::Label pitchLabel;
    juce::Slider zoomSlider;
    juce::Label zoomLabel;
    juce::Slider zPointSlider;
    juce::Label zPointLabel;

    juce::ToggleButton showShellsToggle;
    juce::ToggleButton showPolylineToggle;

    juce::TextButton resetButton;
    juce::TextButton addPointButton;
    juce::TextButton removePointButton;

    juce::Label instructionLabel;
    juce::Label pointCountLabel;
    juce::Label selectedPointLabel;

    void methodNLIButtonClicked();
    void methodBBButtonClicked();
    void methodMSButtonClicked();
    void subdivisionSliderChanged();
    void tValueSliderChanged();
    void yawSliderChanged();
    void pitchSliderChanged();
    void zoomSliderChanged();
    void zPointSliderChanged();
    void showShellsToggleChanged();
    void showPolylineToggleChanged();
    void resetButtonClicked();
    void addPointButtonClicked();
    void removePointButtonClicked();
    void updatePointCountLabel();
    void updateSelectedPointControls(int selectedIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
