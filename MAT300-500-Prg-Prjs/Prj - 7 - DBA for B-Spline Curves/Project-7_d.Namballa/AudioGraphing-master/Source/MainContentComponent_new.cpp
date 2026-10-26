#include "MainContentComponent_new.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    juce::Point<double> lerpPoint(const juce::Point<double>& a,
                                  const juce::Point<double>& b,
                                  double t)
    {
        return { (1.0 - t) * a.x + t * b.x,
                 (1.0 - t) * a.y + t * b.y };
    }

    juce::Colour shellColourForLevel(int level)
    {
        static const juce::Colour palette[] = {
            juce::Colour(0xff8ee36f),
            juce::Colour(0xffffc857),
            juce::Colour(0xffff7f50),
            juce::Colour(0xffc792ea),
            juce::Colour(0xff6fe7ff)
        };
        return palette[(size_t) (level % (int) std::size(palette))];
    }
}

MainContentComponent::MainContentComponent()
{
    setOpaque(true);
    initialiseUi();
    resetProjectVIIConfiguration();
}

void MainContentComponent::initialiseUi()
{
    degreeLabel.setText("Degree", juce::dontSendNotification);
    addAndMakeVisible(degreeLabel);

    degreeSlider.setRange(1, 10, 1);
    degreeSlider.setValue(degree);
    degreeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    degreeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);
    degreeSlider.addListener(this);
    addAndMakeVisible(degreeSlider);

    controlPointLabel.setText("Control Pts", juce::dontSendNotification);
    addAndMakeVisible(controlPointLabel);

    controlPointSlider.setRange(2, 24, 1);
    controlPointSlider.setValue(controlPointCount);
    controlPointSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    controlPointSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);
    controlPointSlider.addListener(this);
    addAndMakeVisible(controlPointSlider);

    intervalLabel.setText("Intervals", juce::dontSendNotification);
    addAndMakeVisible(intervalLabel);

    intervalSlider.setRange(1, 20, 1);
    intervalSlider.setValue(intervalCount);
    intervalSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    intervalSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);
    intervalSlider.addListener(this);
    addAndMakeVisible(intervalSlider);

    tLabel.setText("t", juce::dontSendNotification);
    addAndMakeVisible(tLabel);

    tSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 90, 22);
    tSlider.addListener(this);
    addAndMakeVisible(tSlider);

    resetButton.addListener(this);
    addAndMakeVisible(resetButton);

    openUniformButton.addListener(this);
    addAndMakeVisible(openUniformButton);

    applyKnotButton.addListener(this);
    addAndMakeVisible(applyKnotButton);

    knotLabel.setText("Knot sequence (editable; partial input allowed)", juce::dontSendNotification);
    addAndMakeVisible(knotLabel);

    knotEditor.setMultiLine(true);
    knotEditor.setReadOnly(false);
    knotEditor.setReturnKeyStartsNewLine(false);
    knotEditor.setScrollbarsShown(true);
    knotEditor.addListener(this);
    knotEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12161d));
    knotEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff46505d));
    knotEditor.setColour(juce::TextEditor::textColourId, juce::Colours::whitesmoke);
    addAndMakeVisible(knotEditor);

    knotPresetBox.addItem(knotPresetToString(KnotPreset::openUniform), (int) KnotPreset::openUniform);
    knotPresetBox.addItem(knotPresetToString(KnotPreset::uniform), (int) KnotPreset::uniform);
    knotPresetBox.addItem(knotPresetToString(KnotPreset::custom), (int) KnotPreset::custom);
    knotPresetBox.setSelectedId((int) currentPreset, juce::dontSendNotification);
    knotPresetBox.addListener(this);
    addAndMakeVisible(knotPresetBox);

    modeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(modeLabel);
}

void MainContentComponent::resetProjectVIIConfiguration()
{
    selectedPoint = -1;
    draggingPoint = false;
    currentPreset = KnotPreset::openUniform;

    degree = juce::jlimit(1, 10, degree);
    controlPointCount = juce::jmax(controlPointCount, degree + 1);
    controlPointCount = juce::jlimit(2, 24, controlPointCount);

    const int maxIntervals = juce::jmax(1, 24 - degree);
    intervalCount = juce::jlimit(1, maxIntervals, intervalCount);
    controlPointCount = degree + intervalCount;

    degreeSlider.setValue(degree, juce::dontSendNotification);
    controlPointSlider.setValue(controlPointCount, juce::dontSendNotification);
    intervalSlider.setRange(1, maxIntervals, 1);
    intervalSlider.setValue(intervalCount, juce::dontSendNotification);
    knotPresetBox.setSelectedId((int) currentPreset, juce::dontSendNotification);

    buildDefaultControlPoints();
    rebuildOpenUniformKnots();
    updateTSliderRange(false);
    setStatusMessage("Project VII reset. Drag t to inspect shells and drag control points to edit the curve.", false);
    refreshUiText();
    repaint();
}

void MainContentComponent::rebuildSplineSetup()
{
    const double normalizedT = getCurrentTNormalized();

    degree = juce::jlimit(1, 10, degree);
    controlPointCount = juce::jmax(controlPointCount, degree + 1);
    controlPointCount = juce::jlimit(2, 24, controlPointCount);

    const int maxIntervals = juce::jmax(1, 24 - degree);
    intervalCount = juce::jlimit(1, maxIntervals, intervalCount);
    controlPointCount = degree + intervalCount;
    intervalSlider.setRange(1, maxIntervals, 1);
    controlPointSlider.setValue(controlPointCount, juce::dontSendNotification);

    buildDefaultControlPoints();

    if (currentPreset == KnotPreset::uniform)
        rebuildUniformKnots();
    else
        rebuildOpenUniformKnots();

    selectedPoint = -1;
    draggingPoint = false;
    updateTSliderRange(false);
    tSlider.setValue(juce::jmap(normalizedT,
                                tSlider.getMinimum(),
                                tSlider.getMaximum()),
                     juce::dontSendNotification);
    refreshUiText();
    repaint();
}

void MainContentComponent::buildDefaultControlPoints()
{
    controlPoints.clear();
    controlPoints.reserve((size_t) controlPointCount);

    const double xStart = 0.08;
    const double xEnd = 0.92;

    for (int i = 0; i < controlPointCount; ++i)
    {
        const double a = controlPointCount <= 1 ? 0.5 : (double) i / (double) (controlPointCount - 1);
        const double x = juce::jmap(a, 0.0, 1.0, xStart, xEnd);
        const double phase = (double) i / (double) juce::jmax(1, controlPointCount - 1);
        const double y = 0.52
                         + 0.20 * std::sin(2.0 * juce::MathConstants<double>::pi * phase)
                         + 0.08 * std::cos(5.0 * juce::MathConstants<double>::pi * phase);

        controlPoints.push_back({ x, clamp01(y) });
    }
}

void MainContentComponent::rebuildOpenUniformKnots()
{
    currentKnots.clear();

    const int n = controlPointCount - 1;
    const int p = degree;
    const int m = n + p + 1;
    const int knotCount = m + 1;
    const int interiorCount = n - p;

    currentKnots.reserve((size_t) knotCount);

    for (int i = 0; i <= p; ++i)
        currentKnots.push_back(0.0);

    if (interiorCount > 0)
    {
        for (int j = 1; j <= interiorCount; ++j)
            currentKnots.push_back((double) j / (double) (interiorCount + 1));
    }

    while ((int) currentKnots.size() < knotCount)
        currentKnots.push_back(1.0);
}

void MainContentComponent::rebuildUniformKnots()
{
    currentKnots.clear();

    const int knotCount = controlPointCount + degree + 1;
    currentKnots.reserve((size_t) knotCount);
    for (int i = 0; i < knotCount; ++i)
        currentKnots.push_back((double) i);
}

void MainContentComponent::refreshUiText()
{
    knotEditor.setText(knotSequenceToString(currentKnots), false);
    knotPresetBox.setSelectedId((int) currentPreset, juce::dontSendNotification);

    const CurveInfo info = getCurveInfo();
    const juce::String domainText = info.valid
        ? ("Curve domain: [" + juce::String(info.domainStart, 3) + ", " + juce::String(info.domainEnd, 3) + "]")
        : "Curve domain unavailable";

    modeLabel.setText("Project VII: B-spline curve mode | "
                      + knotPresetToString(currentPreset)
                      + " | " + domainText,
                      juce::dontSendNotification);
}

void MainContentComponent::updateTSliderRange(bool preserveNormalizedPosition)
{
    const double previousNormalized = preserveNormalizedPosition ? getCurrentTNormalized() : 0.5;
    const CurveInfo info = getCurveInfo();

    if (info.valid)
    {
        tSlider.setRange(info.domainStart, info.domainEnd, 0.0001);
        tSlider.setValue(juce::jmap(previousNormalized, info.domainStart, info.domainEnd),
                         juce::dontSendNotification);
    }
    else
    {
        tSlider.setRange(0.0, 1.0, 0.0001);
        tSlider.setValue(0.5, juce::dontSendNotification);
    }
}

bool MainContentComponent::parseKnotEditorText(std::vector<double>& parsedKnots, juce::String& errorMessage) const
{
    parsedKnots.clear();

    juce::String cleaned = knotEditor.getText();
    cleaned = cleaned.replaceCharacters("\n\r\t;", ",,,,");

    juce::StringArray parts;
    parts.addTokens(cleaned, ", ", "");
    parts.trim();
    parts.removeEmptyStrings();

    if (parts.isEmpty())
    {
        errorMessage = "Enter at least one knot value.";
        return false;
    }

    for (const auto& token : parts)
    {
        double value = 0.0;
        if (!token.containsOnly("0123456789+-.eE"))
        {
            errorMessage = "Knot editor contains a non-numeric token: '" + token + "'.";
            return false;
        }

        value = token.getDoubleValue();
        if (!std::isfinite(value))
        {
            errorMessage = "Knot editor contains a non-finite value.";
            return false;
        }

        parsedKnots.push_back(value);
    }

    const int requiredCount = controlPointCount + degree + 1;
    if ((int) parsedKnots.size() > requiredCount)
    {
        errorMessage = "Too many knots. Expected at most " + juce::String(requiredCount) + ".";
        return false;
    }

    std::vector<double> defaults;
    if (currentPreset == KnotPreset::uniform)
    {
        const int knotCount = requiredCount;
        defaults.reserve((size_t) knotCount);
        for (int i = 0; i < knotCount; ++i)
            defaults.push_back((double) i);
    }
    else
    {
        const int n = controlPointCount - 1;
        const int p = degree;
        const int m = n + p + 1;
        const int knotCount = m + 1;
        const int interiorCount = n - p;

        defaults.reserve((size_t) knotCount);
        for (int i = 0; i <= p; ++i)
            defaults.push_back(0.0);
        if (interiorCount > 0)
        {
            for (int j = 1; j <= interiorCount; ++j)
                defaults.push_back((double) j / (double) (interiorCount + 1));
        }
        while ((int) defaults.size() < knotCount)
            defaults.push_back(1.0);
    }

    while ((int) parsedKnots.size() < requiredCount)
    {
        const size_t i = parsedKnots.size();
        const double defaultValue = defaults[i];
        const double minimum = parsedKnots.empty() ? -std::numeric_limits<double>::infinity() : parsedKnots.back();
        parsedKnots.push_back(std::max(defaultValue, minimum));
    }

    return true;
}

bool MainContentComponent::validateKnotSequence(const std::vector<double>& knots, juce::String& errorMessage) const
{
    const int expectedCount = controlPointCount + degree + 1;
    if ((int) knots.size() != expectedCount)
    {
        errorMessage = "Invalid knot count. Expected " + juce::String(expectedCount)
                     + ", got " + juce::String((int) knots.size()) + ".";
        return false;
    }

    for (size_t i = 1; i < knots.size(); ++i)
    {
        if (knots[i] < knots[i - 1])
        {
            errorMessage = "Knot sequence must be nondecreasing.";
            return false;
        }
    }

    const CurveInfo info = getCurveInfo();
    juce::ignoreUnused(info);

    const int n = controlPointCount - 1;
    const int p = degree;
    if (!(knots[(size_t) (n + 1)] > knots[(size_t) p]))
    {
        errorMessage = "The active domain is empty. Need U[n+1] > U[p].";
        return false;
    }

    return true;
}

void MainContentComponent::applyEditedKnotSequence()
{
    std::vector<double> parsedKnots;
    juce::String errorMessage;

    if (!parseKnotEditorText(parsedKnots, errorMessage))
    {
        setStatusMessage(errorMessage, true);
        return;
    }

    if (!validateKnotSequence(parsedKnots, errorMessage))
    {
        setStatusMessage(errorMessage, true);
        return;
    }

    currentKnots = std::move(parsedKnots);
    currentPreset = KnotPreset::custom;
    updateTSliderRange(true);
    refreshUiText();
    setStatusMessage("Custom knot sequence applied successfully.", false);
    repaint();
}

void MainContentComponent::setStatusMessage(const juce::String& message, bool isError)
{
    statusMessage = message;
    statusIsError = isError;
}

void MainContentComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto top = area.removeFromTop(topPanelHeight);

    top.removeFromTop(42);

    auto row1 = top.removeFromTop(30);
    degreeLabel.setBounds(row1.removeFromLeft(56));
    degreeSlider.setBounds(row1.removeFromLeft(240));
    row1.removeFromLeft(12);
    controlPointLabel.setBounds(row1.removeFromLeft(76));
    controlPointSlider.setBounds(row1.removeFromLeft(250));

    top.removeFromTop(8);

    auto row2 = top.removeFromTop(30);
    intervalLabel.setBounds(row2.removeFromLeft(56));
    intervalSlider.setBounds(row2.removeFromLeft(240));
    row2.removeFromLeft(12);
    tLabel.setBounds(row2.removeFromLeft(22));
    tSlider.setBounds(row2.removeFromLeft(300));

    top.removeFromTop(8);

    auto row3 = top.removeFromTop(34);
    resetButton.setBounds(row3.removeFromLeft(170));
    row3.removeFromLeft(8);
    openUniformButton.setBounds(row3.removeFromLeft(190));
    row3.removeFromLeft(8);
    knotPresetBox.setBounds(row3.removeFromLeft(180));
    row3.removeFromLeft(8);
    applyKnotButton.setBounds(row3.removeFromLeft(120));

    top.removeFromTop(8);

    auto row4 = top.removeFromTop(24);
    knotLabel.setBounds(row4.removeFromLeft(360));
    modeLabel.setBounds(row4);

    top.removeFromTop(4);
    knotEditor.setBounds(top.reduced(0, 2));
}

void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0d10));

    auto bounds = getLocalBounds().reduced(10);
    auto controlArea = bounds.removeFromTop(topPanelHeight);
    auto statusArea = bounds.removeFromBottom(statusBarHeight);
    auto canvasArea = bounds;

    drawControls(g, controlArea);
    drawCanvas(g, canvasArea);
    drawStatusBar(g, statusArea);
}

void MainContentComponent::drawControls(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour(0xff1b2028));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff455161));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);

    auto headerArea = area.reduced(12, 10).removeFromTop(34);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("MATH 500 - Project VII: De Boor Algorithm for B-Spline Curves",
                     headerArea,
                     juce::Justification::centredLeft,
                     1);
}

MainContentComponent::PlotArea MainContentComponent::makePlotArea(juce::Rectangle<int> area) const
{
    PlotArea plot;
    plot.rect = area.toFloat().reduced(6.0f);
    return plot;
}

juce::Point<float> MainContentComponent::worldToScreen(const PlotArea& plot, juce::Point<double> p) const
{
    return {
        (float) juce::jmap(p.x, 0.0, 1.0, (double) plot.rect.getX(), (double) plot.rect.getRight()),
        (float) juce::jmap(p.y, 1.0, 0.0, (double) plot.rect.getY(), (double) plot.rect.getBottom())
    };
}

juce::Point<double> MainContentComponent::screenToWorld(const PlotArea& plot, juce::Point<float> p) const
{
    return {
        clamp01(juce::jmap((double) p.x,
                           (double) plot.rect.getX(), (double) plot.rect.getRight(),
                           0.0, 1.0)),
        clamp01(juce::jmap((double) p.y,
                           (double) plot.rect.getY(), (double) plot.rect.getBottom(),
                           1.0, 0.0))
    };
}

void MainContentComponent::drawCanvas(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto plot = makePlotArea(area);

    g.setColour(juce::Colour(0xff10141a));
    g.fillRoundedRectangle(plot.rect, 6.0f);
    g.setColour(juce::Colour(0xff39424f));
    g.drawRoundedRectangle(plot.rect, 6.0f, 1.0f);

    drawGrid(g, plot);
    const ShellData shells = computeShellData(getCurrentT());
    drawCurve(g, plot);
    drawShells(g, plot, shells);
    drawControlPolygon(g, plot, controlPoints, juce::Colour(0xfff7b955), "P", &shells.activeControlIndices);
}

void MainContentComponent::drawGrid(juce::Graphics& g, const PlotArea& plot)
{
    g.setColour(juce::Colour(0xff202833));

    for (int i = 1; i < 10; ++i)
    {
        const float x = juce::jmap((float) i, 0.0f, 10.0f, plot.rect.getX(), plot.rect.getRight());
        const float y = juce::jmap((float) i, 0.0f, 10.0f, plot.rect.getY(), plot.rect.getBottom());
        g.drawLine(x, plot.rect.getY(), x, plot.rect.getBottom(), 0.7f);
        g.drawLine(plot.rect.getX(), y, plot.rect.getRight(), y, 0.7f);
    }
}

void MainContentComponent::drawCurve(juce::Graphics& g, const PlotArea& plot)
{
    const CurveInfo info = getCurveInfo();
    if (!info.valid)
        return;

    juce::Path path;

    for (int sample = 0; sample < curveSamples; ++sample)
    {
        double s = (double) sample / (double) (curveSamples - 1);
        if (sample == curveSamples - 1)
            s = std::nextafter(1.0, 0.0);

        const double t = mapSampleToCurveParameter(s);
        const auto curvePoint = evaluateCurrentCurve(t);
        const auto screen = worldToScreen(plot, curvePoint);

        if (sample == 0)
            path.startNewSubPath(screen);
        else
            path.lineTo(screen);
    }

    g.setColour(juce::Colour(0xff6fe7ff));
    g.strokePath(path, juce::PathStrokeType(2.8f));
}

void MainContentComponent::drawControlPolygon(juce::Graphics& g,
                                              const PlotArea& plot,
                                              const std::vector<juce::Point<double>>& points,
                                              juce::Colour colour,
                                              const juce::String& prefix,
                                              const std::vector<int>* highlightIndices)
{
    if (points.empty())
        return;

    juce::Path polyline;
    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto screen = worldToScreen(plot, points[i]);
        if (i == 0)
            polyline.startNewSubPath(screen);
        else
            polyline.lineTo(screen);
    }

    g.setColour(colour.withAlpha(0.60f));
    g.strokePath(polyline, juce::PathStrokeType(1.5f));

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto screen = worldToScreen(plot, points[i]);
        const bool isSelected = ((int) i == selectedPoint);
        const bool isHighlighted = highlightIndices != nullptr
                                && std::find(highlightIndices->begin(), highlightIndices->end(), (int) i) != highlightIndices->end();
        const float radius = isHighlighted ? pointRadius + 2.5f : pointRadius;

        g.setColour(isSelected ? juce::Colours::yellow
                               : (isHighlighted ? juce::Colours::orange : colour.brighter(0.10f)));
        g.fillEllipse(screen.x - radius,
                      screen.y - radius,
                      2.0f * radius,
                      2.0f * radius);

        if (isHighlighted)
        {
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.drawEllipse(screen.x - radius - 2.0f,
                          screen.y - radius - 2.0f,
                          2.0f * radius + 4.0f,
                          2.0f * radius + 4.0f,
                          1.4f);
        }

        g.setColour(juce::Colours::white.withAlpha(0.95f));
        g.setFont(12.0f);
        g.drawText(prefix + juce::String((int) i),
                   juce::Rectangle<float>(screen.x + 8.0f, screen.y - 10.0f, 40.0f, 18.0f),
                   juce::Justification::centredLeft);
    }
}

void MainContentComponent::drawShells(juce::Graphics& g, const PlotArea& plot, const ShellData& shells)
{
    if (!shells.valid)
        return;

    for (size_t level = 0; level < shells.levels.size(); ++level)
    {
        const auto& pts = shells.levels[level];
        const auto levelColour = shellColourForLevel((int) level);

        if (pts.size() >= 2)
        {
            juce::Path path;
            for (size_t i = 0; i < pts.size(); ++i)
            {
                const auto screen = worldToScreen(plot, pts[i]);
                if (i == 0)
                    path.startNewSubPath(screen);
                else
                    path.lineTo(screen);
            }

            g.setColour(levelColour.withAlpha(level == 0 ? 0.85f : 0.95f));
            g.strokePath(path, juce::PathStrokeType(level == shells.levels.size() - 1 ? 2.8f : 2.0f));
        }

        for (const auto& pt : pts)
        {
            const auto screen = worldToScreen(plot, pt);
            const float radius = (level == shells.levels.size() - 1) ? 5.2f : 4.0f;
            g.setColour(levelColour);
            g.fillEllipse(screen.x - radius, screen.y - radius, 2.0f * radius, 2.0f * radius);
        }
    }

    const auto evaluated = worldToScreen(plot, shells.point);
    g.setColour(juce::Colours::red.brighter(0.2f));
    g.fillEllipse(evaluated.x - 6.5f, evaluated.y - 6.5f, 13.0f, 13.0f);
    g.setColour(juce::Colours::white);
    g.drawEllipse(evaluated.x - 8.5f, evaluated.y - 8.5f, 17.0f, 17.0f, 1.7f);
    g.setFont(13.0f);
    g.drawText("C(t)", juce::Rectangle<float>(evaluated.x + 10.0f, evaluated.y - 10.0f, 48.0f, 18.0f),
               juce::Justification::centredLeft);
}

void MainContentComponent::drawStatusBar(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(statusIsError ? juce::Colour(0xffff8a80) : juce::Colours::lightgrey);
    g.setFont(13.0f);

    const CurveInfo info = getCurveInfo();
    const juce::String text = "Project VII B-spline curve | degree p = "
        + juce::String(degree)
        + " | control points = " + juce::String(controlPointCount)
        + " | intervals = " + juce::String(intervalCount)
        + " | knot count = " + juce::String((int) currentKnots.size())
        + " | t = " + juce::String(getCurrentT(), 4)
        + " | domain = [" + juce::String(info.domainStart, 3) + ", " + juce::String(info.domainEnd, 3) + "]"
        + " | " + statusMessage;

    g.drawText(text, area, juce::Justification::centredLeft);
}

void MainContentComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &degreeSlider)
    {
        degree = (int) degreeSlider.getValue();
        intervalCount = juce::jlimit(1, 24 - degree, intervalCount);
        rebuildSplineSetup();
        return;
    }

    if (slider == &controlPointSlider)
    {
        controlPointCount = (int) controlPointSlider.getValue();
        controlPointCount = juce::jmax(controlPointCount, degree + 1);
        intervalCount = controlPointCount - degree;
        rebuildSplineSetup();
        return;
    }

    if (slider == &intervalSlider)
    {
        intervalCount = (int) intervalSlider.getValue();
        controlPointCount = degree + intervalCount;
        rebuildSplineSetup();
        return;
    }

    if (slider == &tSlider)
    {
        repaint();
        return;
    }
}

void MainContentComponent::buttonClicked(juce::Button* button)
{
    if (button == &resetButton)
    {
        resetProjectVIIConfiguration();
        return;
    }

    if (button == &openUniformButton)
    {
        currentPreset = KnotPreset::openUniform;
        rebuildOpenUniformKnots();
        updateTSliderRange(true);
        refreshUiText();
        setStatusMessage("Open uniform knot sequence restored.", false);
        repaint();
        return;
    }

    if (button == &applyKnotButton)
    {
        applyEditedKnotSequence();
        return;
    }
}

void MainContentComponent::textEditorFocusLost(juce::TextEditor& editor)
{
    if (&editor == &knotEditor)
        applyEditedKnotSequence();
}

void MainContentComponent::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &knotEditor)
        applyEditedKnotSequence();
}

void MainContentComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged != &knotPresetBox)
        return;

    currentPreset = (KnotPreset) knotPresetBox.getSelectedId();

    if (currentPreset == KnotPreset::openUniform)
    {
        rebuildOpenUniformKnots();
        setStatusMessage("Open uniform knot preset applied.", false);
    }
    else if (currentPreset == KnotPreset::uniform)
    {
        rebuildUniformKnots();
        setStatusMessage("Uniform knot preset applied.", false);
    }
    else
    {
        setStatusMessage("Custom preset selected. Edit the knot sequence and press Apply Knots.", false);
    }

    updateTSliderRange(true);
    refreshUiText();
    repaint();
}

int MainContentComponent::findControlPointNear(const PlotArea& plot, juce::Point<float> mousePos) const
{
    const float maxDist2 = (pointRadius + 4.0f) * (pointRadius + 4.0f);

    for (int i = 0; i < (int) controlPoints.size(); ++i)
    {
        const auto screen = worldToScreen(plot, controlPoints[(size_t) i]);
        const auto delta = screen - mousePos;
        if (delta.getDistanceSquaredFromOrigin() <= maxDist2)
            return i;
    }

    return -1;
}

void MainContentComponent::dragActiveControlPointTo(const PlotArea& plot, juce::Point<float> mousePos)
{
    if (selectedPoint < 0 || selectedPoint >= (int) controlPoints.size())
        return;

    controlPoints[(size_t) selectedPoint] = screenToWorld(plot, mousePos);
}

void MainContentComponent::mouseDown(const juce::MouseEvent& e)
{
    const auto plot = makePlotArea(getLocalBounds().reduced(10)
                                       .withTrimmedTop(topPanelHeight)
                                       .withTrimmedBottom(statusBarHeight));

    if (!plot.rect.contains(e.position))
        return;

    selectedPoint = findControlPointNear(plot, e.position);
    draggingPoint = (selectedPoint >= 0);
    repaint();
}

void MainContentComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!draggingPoint)
        return;

    const auto plot = makePlotArea(getLocalBounds().reduced(10)
                                       .withTrimmedTop(topPanelHeight)
                                       .withTrimmedBottom(statusBarHeight));

    dragActiveControlPointTo(plot, e.position);
    repaint();
}

void MainContentComponent::mouseUp(const juce::MouseEvent&)
{
    draggingPoint = false;
    selectedPoint = -1;
    repaint();
}

MainContentComponent::CurveInfo MainContentComponent::getCurveInfo() const
{
    CurveInfo info;

    if (controlPoints.size() < (size_t) (degree + 1))
        return info;

    if (currentKnots.size() < (size_t) (controlPoints.size() + degree + 1))
        return info;

    const int n = (int) controlPoints.size() - 1;
    const int p = degree;

    info.domainStart = currentKnots[(size_t) p];
    info.domainEnd = currentKnots[(size_t) (n + 1)];
    info.valid = (info.domainEnd > info.domainStart);
    return info;
}

double MainContentComponent::mapSampleToCurveParameter(double s) const
{
    const CurveInfo info = getCurveInfo();
    if (!info.valid)
        return 0.0;

    return juce::jmap(s, 0.0, 1.0, info.domainStart, info.domainEnd);
}

double MainContentComponent::getCurrentT() const
{
    const CurveInfo info = getCurveInfo();
    if (!info.valid)
        return 0.0;

    return juce::jlimit(info.domainStart,
                        std::nextafter(info.domainEnd, info.domainStart),
                        tSlider.getValue());
}

double MainContentComponent::getCurrentTNormalized() const
{
    const CurveInfo info = getCurveInfo();
    if (!info.valid || !(info.domainEnd > info.domainStart))
        return 0.5;

    return clamp01((tSlider.getValue() - info.domainStart) / (info.domainEnd - info.domainStart));
}

juce::Point<double> MainContentComponent::evaluateCurrentCurve(double t) const
{
    return evaluateDeBoorCurve(controlPoints, currentKnots, degree, t);
}

MainContentComponent::ShellData MainContentComponent::computeShellData(double t) const
{
    ShellData shellData;

    if (controlPoints.empty())
        return shellData;

    const int n = (int) controlPoints.size() - 1;
    const int p = degree;

    if (n < p)
        return shellData;

    if ((int) currentKnots.size() != n + p + 2)
        return shellData;

    const double left = currentKnots[(size_t) p];
    const double right = currentKnots[(size_t) (n + 1)];
    if (!(right > left))
        return shellData;

    t = juce::jlimit(left, std::nextafter(right, left), t);
    const int k = findKnotSpan(currentKnots, p, n, t);

    std::vector<juce::Point<double>> d((size_t) (p + 1));
    shellData.activeControlIndices.reserve((size_t) (p + 1));
    for (int j = 0; j <= p; ++j)
    {
        d[(size_t) j] = controlPoints[(size_t) (k - p + j)];
        shellData.activeControlIndices.push_back(k - p + j);
    }

    shellData.levels.push_back(d);

    for (int r = 1; r <= p; ++r)
    {
        std::vector<juce::Point<double>> nextLevel;
        nextLevel.reserve((size_t) (p - r + 1));

        for (int j = r; j <= p; ++j)
        {
            const int i = k - p + j;
            const double denom = currentKnots[(size_t) (i + p - r + 1)] - currentKnots[(size_t) i];

            double alpha = 0.0;
            if (std::abs(denom) > 1.0e-12)
                alpha = (t - currentKnots[(size_t) i]) / denom;

            d[(size_t) j] = lerpPoint(d[(size_t) (j - 1)], d[(size_t) j], alpha);
            nextLevel.push_back(d[(size_t) j]);
        }

        shellData.levels.push_back(nextLevel);
    }

    shellData.point = d[(size_t) p];
    shellData.valid = true;
    return shellData;
}

int MainContentComponent::findKnotSpan(const std::vector<double>& knots,
                                       int splineDegree,
                                       int lastControlIndex,
                                       double t) const
{
    if (t >= knots[(size_t) (lastControlIndex + 1)])
        return lastControlIndex;

    int low = splineDegree;
    int high = lastControlIndex + 1;
    int mid = (low + high) / 2;

    while (t < knots[(size_t) mid] || t >= knots[(size_t) (mid + 1)])
    {
        if (t < knots[(size_t) mid])
            high = mid;
        else
            low = mid;

        mid = (low + high) / 2;
    }

    return mid;
}

juce::Point<double> MainContentComponent::evaluateDeBoorCurve(const std::vector<juce::Point<double>>& pts,
                                                              const std::vector<double>& knots,
                                                              int splineDegree,
                                                              double t) const
{
    if (pts.empty())
        return { 0.0, 0.0 };

    const int n = (int) pts.size() - 1;
    const int p = splineDegree;

    if (n < p)
        return pts.back();

    if ((int) knots.size() != n + p + 2)
        return pts.back();

    const double left = knots[(size_t) p];
    const double right = knots[(size_t) (n + 1)];
    if (!(right > left))
        return pts.back();

    t = juce::jlimit(left, std::nextafter(right, left), t);

    const int k = findKnotSpan(knots, p, n, t);

    std::vector<juce::Point<double>> d((size_t) (p + 1));
    for (int j = 0; j <= p; ++j)
        d[(size_t) j] = pts[(size_t) (k - p + j)];

    for (int r = 1; r <= p; ++r)
    {
        for (int j = p; j >= r; --j)
        {
            const int i = k - p + j;
            const double denom = knots[(size_t) (i + p - r + 1)] - knots[(size_t) i];

            double alpha = 0.0;
            if (std::abs(denom) > 1.0e-12)
                alpha = (t - knots[(size_t) i]) / denom;

            d[(size_t) j] = lerpPoint(d[(size_t) (j - 1)], d[(size_t) j], alpha);
        }
    }

    return d[(size_t) p];
}

double MainContentComponent::clamp01(double value)
{
    return juce::jlimit(0.0, 1.0, value);
}

juce::String MainContentComponent::knotSequenceToString(const std::vector<double>& knots)
{
    juce::StringArray parts;
    for (double knot : knots)
        parts.add(juce::String(knot, 3));
    return parts.joinIntoString(", ");
}

juce::String MainContentComponent::knotPresetToString(KnotPreset preset)
{
    switch (preset)
    {
        case KnotPreset::openUniform: return "Preset: Open Uniform (Clamped)";
        case KnotPreset::uniform:     return "Preset: Uniform";
        case KnotPreset::custom:      return "Preset: Custom";
        default:                      return "Preset: Unknown";
    }
}
