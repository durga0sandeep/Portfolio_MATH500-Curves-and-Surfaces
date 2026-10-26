/*
  ==============================================================================

    MainContentComponent.h
    MATH 500 - Project 2

  ==============================================================================
*/

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
    // Bezier components
    BezierComponent bezierView;
    
    // Method selection buttons
    juce::TextButton methodNLIButton;
    juce::TextButton methodBBButton;
    juce::TextButton methodMSButton;
    
    // Control sliders
    juce::Slider subdivisionSlider;
    juce::Label subdivisionLabel;
    juce::Slider tValueSlider;
    juce::Label tValueLabel;
    
    // Toggle buttons
    juce::ToggleButton showShellsToggle;
    juce::ToggleButton showPolylineToggle;
    
    // Control point management
    juce::TextButton resetButton;
    juce::TextButton addPointButton;
    juce::TextButton removePointButton;
    juce::Label instructionLabel;
    juce::Label pointCountLabel;
    
    // Button callbacks
    void methodNLIButtonClicked();
    void methodBBButtonClicked();
    void methodMSButtonClicked();
    void subdivisionSliderChanged();
    void tValueSliderChanged();
    void showShellsToggleChanged();
    void showPolylineToggleChanged();
    void resetButtonClicked();
    void addPointButtonClicked();
    void removePointButtonClicked();
    void updatePointCountLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
