#include "MainContentComponent_new.h"

MainContentComponent::MainContentComponent()
{
    addAndMakeVisible(methodNLIButton);
    methodNLIButton.setButtonText("NLI / De Casteljau");
    methodNLIButton.onClick = [this] { methodNLIButtonClicked(); };

    addAndMakeVisible(methodBBButton);
    methodBBButton.setButtonText("Bernstein Basis");
    methodBBButton.onClick = [this] { methodBBButtonClicked(); };

    addAndMakeVisible(methodMSButton);
    methodMSButton.setButtonText("Midpoint Subdivision");
    methodMSButton.onClick = [this] { methodMSButtonClicked(); };

    addAndMakeVisible(subdivisionLabel);
    subdivisionLabel.setText("Subdivision k", juce::dontSendNotification);
    subdivisionLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(subdivisionSlider);
    subdivisionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    subdivisionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    subdivisionSlider.setRange(1, 8, 1);
    subdivisionSlider.setValue(3);
    subdivisionSlider.onValueChange = [this] { subdivisionSliderChanged(); };

    addAndMakeVisible(tValueLabel);
    tValueLabel.setText("t value", juce::dontSendNotification);
    tValueLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(tValueSlider);
    tValueSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tValueSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    tValueSlider.setRange(0.0, 1.0, 0.001);
    tValueSlider.setValue(0.5);
    tValueSlider.onValueChange = [this] { tValueSliderChanged(); };

    addAndMakeVisible(yawLabel);
    yawLabel.setText("View yaw", juce::dontSendNotification);
    yawLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(yawSlider);
    yawSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    yawSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    yawSlider.setRange(-3.14159, 3.14159, 0.001);
    yawSlider.setValue(bezierView.getCameraYaw());
    yawSlider.onValueChange = [this] { yawSliderChanged(); };

    addAndMakeVisible(pitchLabel);
    pitchLabel.setText("View pitch", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(pitchSlider);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    pitchSlider.setRange(-1.45, 1.45, 0.001);
    pitchSlider.setValue(bezierView.getCameraPitch());
    pitchSlider.onValueChange = [this] { pitchSliderChanged(); };

    addAndMakeVisible(zoomLabel);
    zoomLabel.setText("Zoom", juce::dontSendNotification);
    zoomLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(zoomSlider);
    zoomSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    zoomSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    zoomSlider.setRange(0.35, 3.5, 0.001);
    zoomSlider.setValue(bezierView.getCameraZoom());
    zoomSlider.onValueChange = [this] { zoomSliderChanged(); };

    addAndMakeVisible(zPointLabel);
    zPointLabel.setText("Selected point Z", juce::dontSendNotification);
    zPointLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(zPointSlider);
    zPointSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    zPointSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    zPointSlider.setRange(-4.0, 4.0, 0.01);
    zPointSlider.setValue(0.0);
    zPointSlider.setEnabled(false);
    zPointSlider.onValueChange = [this] { zPointSliderChanged(); };

    addAndMakeVisible(showShellsToggle);
    showShellsToggle.setButtonText("Show shells (NLI)");
    showShellsToggle.setToggleState(true, juce::dontSendNotification);
    showShellsToggle.onClick = [this] { showShellsToggleChanged(); };

    addAndMakeVisible(showPolylineToggle);
    showPolylineToggle.setButtonText("Show control polyline");
    showPolylineToggle.setToggleState(true, juce::dontSendNotification);
    showPolylineToggle.onClick = [this] { showPolylineToggleChanged(); };

    addAndMakeVisible(resetButton);
    resetButton.setButtonText("Reset Default");
    resetButton.onClick = [this] { resetButtonClicked(); };

    addAndMakeVisible(addPointButton);
    addPointButton.setButtonText("Add Point (+)");
    addPointButton.onClick = [this] { addPointButtonClicked(); };

    addAndMakeVisible(removePointButton);
    removePointButton.setButtonText("Remove Point (-)");
    removePointButton.onClick = [this] { removePointButtonClicked(); };

    addAndMakeVisible(instructionLabel);
    instructionLabel.setJustificationType(juce::Justification::centredLeft);
    instructionLabel.setText("Drag points in the view plane. Hold Shift while dragging to move in Z. Double-click inside the view to add a point.", juce::dontSendNotification);
    instructionLabel.setFont(juce::Font(13.0f));

    addAndMakeVisible(pointCountLabel);
    pointCountLabel.setJustificationType(juce::Justification::centredRight);
    pointCountLabel.setFont(juce::Font(13.0f, juce::Font::bold));

    addAndMakeVisible(selectedPointLabel);
    selectedPointLabel.setJustificationType(juce::Justification::centredLeft);
    selectedPointLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    selectedPointLabel.setText("Selected point: none", juce::dontSendNotification);

    addAndMakeVisible(bezierView);

    bezierView.onControlPointsChanged = [this]() { updatePointCountLabel(); };
    bezierView.onSelectedPointChanged = [this](int selectedIndex) { updateSelectedPointControls(selectedIndex); };

    methodNLIButtonClicked();
    updatePointCountLabel();
    updateSelectedPointControls(-1);

    setSize(1280, 860);
}

void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colour(0xff16324f));
    g.fillRect(0, 0, getWidth(), 44);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("MATH 500 - Project 8: 3D Bezier Curve (3D version of Project II)",
               0, 0, getWidth(), 44, juce::Justification::centred);
}

void MainContentComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50);

    auto controls = bounds.removeFromTop(210).reduced(10, 6);

    auto row1 = controls.removeFromTop(30);
    methodNLIButton.setBounds(row1.removeFromLeft(190));
    row1.removeFromLeft(10);
    methodBBButton.setBounds(row1.removeFromLeft(170));
    row1.removeFromLeft(10);
    methodMSButton.setBounds(row1.removeFromLeft(210));
    row1.removeFromLeft(20);
    pointCountLabel.setBounds(row1.removeFromLeft(180));
    row1.removeFromLeft(10);
    selectedPointLabel.setBounds(row1.removeFromLeft(220));

    controls.removeFromTop(8);

    auto row2 = controls.removeFromTop(26);
    subdivisionLabel.setBounds(row2.removeFromLeft(110));
    subdivisionSlider.setBounds(row2.removeFromLeft(240));
    row2.removeFromLeft(16);
    tValueLabel.setBounds(row2.removeFromLeft(80));
    tValueSlider.setBounds(row2.removeFromLeft(240));
    row2.removeFromLeft(16);
    showShellsToggle.setBounds(row2.removeFromLeft(170));

    controls.removeFromTop(8);

    auto row3 = controls.removeFromTop(26);
    yawLabel.setBounds(row3.removeFromLeft(80));
    yawSlider.setBounds(row3.removeFromLeft(250));
    row3.removeFromLeft(16);
    pitchLabel.setBounds(row3.removeFromLeft(80));
    pitchSlider.setBounds(row3.removeFromLeft(250));
    row3.removeFromLeft(16);
    showPolylineToggle.setBounds(row3.removeFromLeft(180));

    controls.removeFromTop(8);

    auto row4 = controls.removeFromTop(26);
    zoomLabel.setBounds(row4.removeFromLeft(60));
    zoomSlider.setBounds(row4.removeFromLeft(250));
    row4.removeFromLeft(16);
    zPointLabel.setBounds(row4.removeFromLeft(120));
    zPointSlider.setBounds(row4.removeFromLeft(250));

    controls.removeFromTop(8);

    auto row5 = controls.removeFromTop(28);
    addPointButton.setBounds(row5.removeFromLeft(130));
    row5.removeFromLeft(10);
    removePointButton.setBounds(row5.removeFromLeft(140));
    row5.removeFromLeft(10);
    resetButton.setBounds(row5.removeFromLeft(140));
    row5.removeFromLeft(20);
    instructionLabel.setBounds(row5.removeFromLeft(getWidth() - 520));

    bounds.reduce(10, 8);
    bezierView.setBounds(bounds);
}

void MainContentComponent::methodNLIButtonClicked()
{
    bezierView.setComputationMethod(BezierComponent::NESTED_LINEAR_INTERPOLATION);
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colours::steelblue);
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff86c47a));
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe38f73));
    repaint();
}

void MainContentComponent::methodBBButtonClicked()
{
    bezierView.setComputationMethod(BezierComponent::BERNSTEIN_POLYNOMIALS);
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8db5da));
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe38f73));
    repaint();
}

void MainContentComponent::methodMSButtonClicked()
{
    bezierView.setComputationMethod(BezierComponent::MIDPOINT_SUBDIVISION);
    methodNLIButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8db5da));
    methodBBButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff86c47a));
    methodMSButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orangered);
    repaint();
}

void MainContentComponent::subdivisionSliderChanged()
{
    bezierView.setSubdivisionLevel(static_cast<int>(subdivisionSlider.getValue()));
}

void MainContentComponent::tValueSliderChanged()
{
    bezierView.setTValue(static_cast<float>(tValueSlider.getValue()));
}

void MainContentComponent::yawSliderChanged()
{
    bezierView.setCameraYaw(static_cast<float>(yawSlider.getValue()));
}

void MainContentComponent::pitchSliderChanged()
{
    bezierView.setCameraPitch(static_cast<float>(pitchSlider.getValue()));
}

void MainContentComponent::zoomSliderChanged()
{
    bezierView.setCameraZoom(static_cast<float>(zoomSlider.getValue()));
}

void MainContentComponent::zPointSliderChanged()
{
    if (zPointSlider.isEnabled())
        bezierView.setSelectedPointZ(static_cast<float>(zPointSlider.getValue()));
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
}

void MainContentComponent::addPointButtonClicked()
{
    bezierView.addControlPoint({ 0.0f, 0.0f, 0.0f });
}

void MainContentComponent::removePointButtonClicked()
{
    bezierView.removeLastControlPoint();
}

void MainContentComponent::updatePointCountLabel()
{
    const int numPoints = bezierView.getNumControlPoints();
    pointCountLabel.setText("Control Points: " + juce::String(numPoints) + " / 20", juce::dontSendNotification);
    addPointButton.setEnabled(numPoints < 20);
    removePointButton.setEnabled(numPoints > 2);
}

void MainContentComponent::updateSelectedPointControls(int selectedIndex)
{
    if (selectedIndex < 0)
    {
        selectedPointLabel.setText("Selected point: none", juce::dontSendNotification);
        zPointSlider.setEnabled(false);
        zPointSlider.setValue(0.0, juce::dontSendNotification);
        return;
    }

    selectedPointLabel.setText("Selected point: P" + juce::String(selectedIndex), juce::dontSendNotification);
    zPointSlider.setEnabled(true);
    zPointSlider.setValue(bezierView.getSelectedPointZ(), juce::dontSendNotification);
}
