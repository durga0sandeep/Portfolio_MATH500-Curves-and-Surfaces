/*
  ==============================================================================
    MainContentComponent.cpp
    MATH 500 - Project 2
  ==============================================================================
*/

#include "MainContentComponent_new.h"

MainContentComponent::MainContentComponent()
{
    // Point count label
    addAndMakeVisible(&pointCountLabel);
    pointCountLabel.setJustificationType(juce::Justification::centred);
    pointCountLabel.setFont(juce::Font(12.0f));
    updatePointCountLabel();

    // Method selection buttons
    addAndMakeVisible(&methodNLIButton);
    methodNLIButton.setButtonText("Nested Linear Interpolation");
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    methodNLIButton.onClick = [this] { methodNLIButtonClicked(); };
    
    addAndMakeVisible(&methodBBButton);
    methodBBButton.setButtonText("Bernstein Basis");
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgreen);
    methodBBButton.onClick = [this] { methodBBButtonClicked(); };
    
    addAndMakeVisible(&methodMSButton);
    methodMSButton.setButtonText("Midpoint Subdivision");
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightcoral);
    methodMSButton.onClick = [this] { methodMSButtonClicked(); };
    
    // Subdivision level slider
    addAndMakeVisible(&subdivisionLabel);
    subdivisionLabel.setText("Subdivision Level (k):", juce::dontSendNotification);
    subdivisionLabel.setJustificationType(juce::Justification::centredRight);
    subdivisionLabel.attachToComponent(&subdivisionSlider, true);
    
    addAndMakeVisible(&subdivisionSlider);
    subdivisionSlider.setRange(1, 10, 1);
    subdivisionSlider.setValue(3);
    subdivisionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    subdivisionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    subdivisionSlider.onValueChange = [this] { subdivisionSliderChanged(); };
    
    // t-value slider
    addAndMakeVisible(&tValueLabel);
    tValueLabel.setText("t value (0-1):", juce::dontSendNotification);
    tValueLabel.setJustificationType(juce::Justification::centredRight);
    tValueLabel.attachToComponent(&tValueSlider, true);
    
    addAndMakeVisible(&tValueSlider);
    tValueSlider.setRange(0.0, 1.0, 0.01);
    tValueSlider.setValue(0.5);
    tValueSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tValueSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    tValueSlider.onValueChange = [this] { tValueSliderChanged(); };
    
    // Toggle buttons
    addAndMakeVisible(&showShellsToggle);
    showShellsToggle.setButtonText("Show Shells (NLI)");
    showShellsToggle.setToggleState(true, juce::dontSendNotification);
    showShellsToggle.onClick = [this] { showShellsToggleChanged(); };
    
    addAndMakeVisible(&showPolylineToggle);
    showPolylineToggle.setButtonText("Show Control Polyline");
    showPolylineToggle.setToggleState(true, juce::dontSendNotification);
    showPolylineToggle.onClick = [this] { showPolylineToggleChanged(); };
    
    // Reset button
    addAndMakeVisible(&resetButton);
    resetButton.setButtonText("Reset to Default");
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    resetButton.onClick = [this] { resetButtonClicked(); };
    
    // Add point button
    addAndMakeVisible(&addPointButton);
    addPointButton.setButtonText("Add Point (+)");
    addPointButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgreen);
    addPointButton.onClick = [this] { addPointButtonClicked(); };
    
    // Remove point button
    addAndMakeVisible(&removePointButton);
    removePointButton.setButtonText("Remove Point (-)");
    removePointButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightcoral);
    removePointButton.onClick = [this] { removePointButtonClicked(); };
    
    // Instruction label
    addAndMakeVisible(&instructionLabel);
    instructionLabel.setText("Instructions: Drag to move points | Double-click to add | Use buttons to add/remove", 
                            juce::dontSendNotification);
    instructionLabel.setJustificationType(juce::Justification::centred);
    instructionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    
    // Point count label
    addAndMakeVisible(&pointCountLabel);
    pointCountLabel.setJustificationType(juce::Justification::centred);
    pointCountLabel.setFont(juce::Font(12.0f));
    updatePointCountLabel();
    
    // Bezier view
    addAndMakeVisible(&bezierView);
    
    // Set up callback for point count updates
    bezierView.onControlPointsChanged = [this]() { updatePointCountLabel(); };
    
    // Set initial method to NLI
    methodNLIButtonClicked();
    
    setSize(1200, 800);
}

void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    // Draw a nice header
    g.setColour(juce::Colours::darkblue);
    g.fillRect(0, 0, getWidth(), 40);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("MATH 500 - Project 2: De Casteljau Algorithm for Bezier Curves", 
               0, 0, getWidth(), 40, juce::Justification::centred);

}

void MainContentComponent::resized()
{
    auto bounds = getLocalBounds();

    // Header space
    bounds.removeFromTop(50);

    // Top controls area (increased height for extra button row)
    auto controlsArea = bounds.removeFromTop(140);
    controlsArea.reduce(10, 5);

    // Method buttons (row 1)
    auto methodRow = controlsArea.removeFromTop(30);
    methodRow.removeFromLeft(5);
    methodNLIButton.setBounds(methodRow.removeFromLeft(200));
    methodRow.removeFromLeft(10);
    methodBBButton.setBounds(methodRow.removeFromLeft(200));
    methodRow.removeFromLeft(10);
    methodMSButton.setBounds(methodRow.removeFromLeft(200));

    methodRow.removeFromLeft(50);  // spacing
    pointCountLabel.setBounds(methodRow.removeFromLeft(180)); 

    controlsArea.removeFromTop(10);

    // Sliders and toggles (row 2)
    auto sliderRow = controlsArea.removeFromTop(25);
    sliderRow.removeFromLeft(160);  // Space for label
    subdivisionSlider.setBounds(sliderRow.removeFromLeft(200));
    sliderRow.removeFromLeft(40);
    showShellsToggle.setBounds(sliderRow.removeFromLeft(180));

    controlsArea.removeFromTop(5);

    // Row 3
    auto sliderRow2 = controlsArea.removeFromTop(25);
    sliderRow2.removeFromLeft(160);  // Space for label
    tValueSlider.setBounds(sliderRow2.removeFromLeft(200));
    sliderRow2.removeFromLeft(40);
    showPolylineToggle.setBounds(sliderRow2.removeFromLeft(180));

    controlsArea.removeFromTop(5);

    // Row 4 - Point management buttons
    auto buttonRow = controlsArea.removeFromTop(25);
    buttonRow.removeFromLeft(160);  // Align with sliders
    addPointButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(10);
    removePointButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(10);
    resetButton.setBounds(buttonRow.removeFromLeft(140));

    controlsArea.removeFromTop(5);

    // Instruction and point count row
    auto infoRow = controlsArea.removeFromTop(25);
    auto leftInfo = infoRow.removeFromLeft(getWidth() * 0.7f);
    instructionLabel.setBounds(leftInfo);

    // Bezier view takes the rest
    bounds.reduce(10, 5);
    bezierView.setBounds(bounds);
}
// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

void MainContentComponent::methodNLIButtonClicked()
{
    bezierView.setComputationMethod(BezierComponent::NESTED_LINEAR_INTERPOLATION);
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgreen);
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightcoral);
    repaint();
}

void MainContentComponent::methodBBButtonClicked()
{
    bezierView.setComputationMethod(BezierComponent::BERNSTEIN_POLYNOMIALS);
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightcoral);
    repaint();
}

void MainContentComponent::methodMSButtonClicked()
{
    bezierView.setComputationMethod(BezierComponent::MIDPOINT_SUBDIVISION);
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgreen);
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    repaint();
}

void MainContentComponent::subdivisionSliderChanged()
{
    bezierView.setSubdivisionLevel((int)subdivisionSlider.getValue());
}

void MainContentComponent::tValueSliderChanged()
{
    bezierView.setTValue((float)tValueSlider.getValue());
}

void MainContentComponent::showShellsToggleChanged()
{
    bezierView.toggleShowShells(showShellsToggle.getToggleState());
}

void MainContentComponent::showPolylineToggleChanged()
{
    bezierView.toggleShowControlPolyline(showPolylineToggle.getToggleState());
}

void MainContentComponent::resetButtonClicked()
{
    bezierView.clearControlPoints();
    updatePointCountLabel();
}

void MainContentComponent::addPointButtonClicked()
{
    // Add a point in the center of the drawing area
    auto bounds = bezierView.getBounds();
    float centerX = bounds.getWidth() / 2.0f;
    float centerY = bounds.getHeight() / 2.0f;
    
    // Add some randomness so multiple adds don't stack exactly
    float offsetX = (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 100.0f;
    float offsetY = (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 100.0f;
    
    bezierView.addControlPoint(juce::Point<float>(centerX + offsetX, centerY + offsetY));
    updatePointCountLabel();
}

void MainContentComponent::removePointButtonClicked()
{
    bezierView.removeLastControlPoint();
    updatePointCountLabel();
}

void MainContentComponent::updatePointCountLabel()
{
    int numPoints = bezierView.getNumControlPoints();
    juce::String text = "Control Points: " + juce::String(numPoints) + " / 20";
    pointCountLabel.setText(text, juce::dontSendNotification);
    
    // Enable/disable buttons based on point count
    addPointButton.setEnabled(numPoints < 20);
    removePointButton.setEnabled(numPoints > 2);
}












