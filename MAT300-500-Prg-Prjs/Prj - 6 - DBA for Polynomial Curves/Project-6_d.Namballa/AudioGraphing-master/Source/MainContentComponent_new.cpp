#include "MainContentComponent_new.h"

#include <algorithm>
#include <cmath>

namespace
{
    juce::Point<double> lerpPoint(const juce::Point<double>& a,
                                  const juce::Point<double>& b,
                                  double t)
    {
        return { (1.0 - t) * a.x + t * b.x,
                 (1.0 - t) * a.y + t * b.y };
    }
}

MainContentComponent::MainContentComponent()
{
    setOpaque(true);
    initialiseUi();
    resetToDefaultBezierConfiguration();
}

void MainContentComponent::initialiseUi()
{
    degreeLabel.setText("Degree", juce::dontSendNotification);
    addAndMakeVisible(degreeLabel);

    degreeSlider.setRange(1, 20, 1);
    degreeSlider.setValue(degree);
    degreeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    degreeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);
    degreeSlider.addListener(this);
    addAndMakeVisible(degreeSlider);

    resetBezierButton.addListener(this);
    addAndMakeVisible(resetBezierButton);

    usePartVIKnotsButton.addListener(this);
    addAndMakeVisible(usePartVIKnotsButton);

    showBezierModeButton.addListener(this);
    addAndMakeVisible(showBezierModeButton);

    knotLabel.setText("Current knot sequence", juce::dontSendNotification);
    addAndMakeVisible(knotLabel);

    knotEditor.setMultiLine(true);
    knotEditor.setReadOnly(true);
    knotEditor.setScrollbarsShown(true);
    knotEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12161d));
    knotEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff46505d));
    knotEditor.setColour(juce::TextEditor::textColourId, juce::Colours::whitesmoke);
    addAndMakeVisible(knotEditor);

    modeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(modeLabel);
}

void MainContentComponent::resetToDefaultBezierConfiguration()
{
    usingPartVIKnots = false;
    selectedPoint = -1;
    draggingPoint = false;

    bezierPoints.clear();
    deBoorPoints.clear();

    const int pointCount = degree + 1;
    bezierPoints.reserve((size_t) pointCount);

    for (int i = 0; i < pointCount; ++i)
    {
        const double x = pointCount == 1 ? 0.5 : (double) i / (double) (pointCount - 1);
        const double wave = std::sin((double) i * juce::MathConstants<double>::pi / (double) juce::jmax(1, pointCount - 1));
        const double y = 0.55 + 0.18 * ((i % 2 == 0) ? wave : -wave);
        bezierPoints.push_back({ x, y });
    }

    rebuildBezierKnots();
    refreshKnotEditor();
    repaint();
}

void MainContentComponent::rebuildBezierKnots()
{
    currentKnots.assign((size_t) (2 * degree + 2), 0.0);
    for (int i = degree + 1; i < 2 * degree + 2; ++i)
        currentKnots[(size_t) i] = 1.0;
}

std::vector<double> MainContentComponent::buildPartVINewDefaultKnots() const
{
    std::vector<double> knots;
    knots.reserve((size_t) (2 * degree + 2));

    for (int i = 0; i < degree; ++i)
        knots.push_back(-((double) degree - (double) i) / 10.0);

    knots.push_back(0.0);
    knots.push_back(1.0);

    for (int i = 1; i <= degree; ++i)
        knots.push_back(1.0 + (double) i / 10.0);

    return knots;
}

void MainContentComponent::switchToPartVIKnotMode()
{
    currentKnots = buildPartVINewDefaultKnots();
    rebuildDeBoorPointsFromBezier();
    usingPartVIKnots = true;
    selectedPoint = -1;
    draggingPoint = false;
    refreshKnotEditor();
    repaint();
}

void MainContentComponent::switchToBezierMode()
{
    usingPartVIKnots = false;
    selectedPoint = -1;
    draggingPoint = false;
    rebuildBezierKnots();
    refreshKnotEditor();
    repaint();
}

void MainContentComponent::rebuildDeBoorPointsFromBezier()
{
    std::vector<juce::Point<double>> generated;
    generated.reserve((size_t) (degree + 1));

    for (int i = 0; i <= degree; ++i)
    {
        std::vector<double> u;
        u.reserve((size_t) degree);
        for (int k = 1; k <= degree; ++k)
            u.push_back(currentKnots[(size_t) (i + k)]);

        generated.push_back(evaluateBlossom(u));
    }

    deBoorPoints = std::move(generated);
}

void MainContentComponent::refreshKnotEditor()
{
    knotEditor.setText(knotSequenceToString(currentKnots), false);
    modeLabel.setText(usingPartVIKnots
                          ? "Mode: Part VI knot sequence active, drag Q-points"
                          : "Mode: Bezier knot sequence active, drag P-points",
                      juce::dontSendNotification);
}

void MainContentComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto top = area.removeFromTop(topPanelHeight);

    auto row1 = top.removeFromTop(30);
    degreeLabel.setBounds(row1.removeFromLeft(52));
    degreeSlider.setBounds(row1.removeFromLeft(260));
    row1.removeFromLeft(12);
    modeLabel.setBounds(row1.removeFromLeft(430));

    auto row2 = top.removeFromTop(32);
    resetBezierButton.setBounds(row2.removeFromLeft(180));
    row2.removeFromLeft(10);
    usePartVIKnotsButton.setBounds(row2.removeFromLeft(245));
    row2.removeFromLeft(10);
    showBezierModeButton.setBounds(row2.removeFromLeft(160));

    top.removeFromTop(8);
    auto row3 = top;
    knotLabel.setBounds(row3.removeFromTop(18));
    knotEditor.setBounds(row3.reduced(0, 2));
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

    /*
    auto titleArea = area.reduced(10).removeFromTop(18);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("MATH 500 - Part VI: De Boor Algorithm for Polynomial Curves",
               titleArea,
               juce::Justification::centredLeft);
    */
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
    drawCurve(g, plot);

    drawControlPolygon(g,
                       plot,
                       bezierPoints,
                       usingPartVIKnots ? juce::Colour(0x666f86a5) : juce::Colour(0xfff7b955),
                       !usingPartVIKnots,
                       "P");

    if (!deBoorPoints.empty())
    {
        drawControlPolygon(g,
                           plot,
                           deBoorPoints,
                           usingPartVIKnots ? juce::Colour(0xff7be0c3) : juce::Colour(0x664fc3f7),
                           usingPartVIKnots,
                           "Q");
    }
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
    juce::Path path;

    for (int sample = 0; sample < curveSamples; ++sample)
    {
        double t = (double) sample / (double) (curveSamples - 1);
        if (sample == curveSamples - 1)
            t = std::nextafter(1.0, 0.0);

        const auto curvePoint = evaluateCurrentCurve(t);
        const auto screen = worldToScreen(plot, curvePoint);

        if (sample == 0)
            path.startNewSubPath(screen);
        else
            path.lineTo(screen);
    }

    g.setColour(usingPartVIKnots ? juce::Colour(0xff6fe7ff) : juce::Colour(0xff4fc3f7));
    g.strokePath(path, juce::PathStrokeType(2.8f));
}

void MainContentComponent::drawControlPolygon(juce::Graphics& g,
                                              const PlotArea& plot,
                                              const std::vector<juce::Point<double>>& points,
                                              juce::Colour colour,
                                              bool active,
                                              const juce::String& prefix)
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

    g.setColour(colour.withAlpha(active ? 0.95f : 0.45f));
    g.strokePath(polyline, juce::PathStrokeType(active ? 1.9f : 1.3f));

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto screen = worldToScreen(plot, points[i]);
        const bool isSelected = active && ((int) i == selectedPoint);

        g.setColour(isSelected ? juce::Colours::yellow : colour.brighter(active ? 0.15f : 0.0f));
        g.fillEllipse(screen.x - pointRadius,
                      screen.y - pointRadius,
                      2.0f * pointRadius,
                      2.0f * pointRadius);

        g.setColour(juce::Colours::white.withAlpha(active ? 0.95f : 0.65f));
        g.setFont(12.0f);
        g.drawText(prefix + juce::String((int) i),
                   juce::Rectangle<float>(screen.x + 8.0f, screen.y - 10.0f, 40.0f, 18.0f),
                   juce::Justification::centredLeft);
    }
}

void MainContentComponent::drawStatusBar(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(13.0f);

    const auto& activePoints = getActiveControlPoints();
    const juce::String text = "Active control set: "
        + juce::String(usingPartVIKnots ? "De Boor Q-points" : "Bezier P-points")
        + "    Degree d = " + juce::String(degree)
        + "    Control points = " + juce::String((int) activePoints.size())
        + "    Curve domain = [0, 1]"
        + "    Click and drag the active points directly in the canvas.";

    g.drawText(text, area, juce::Justification::centredLeft);
}

void MainContentComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &degreeSlider)
    {
        degree = (int) degreeSlider.getValue();
        resetToDefaultBezierConfiguration();
    }
}

void MainContentComponent::buttonClicked(juce::Button* button)
{
    if (button == &resetBezierButton)
    {
        resetToDefaultBezierConfiguration();
        return;
    }

    if (button == &usePartVIKnotsButton)
    {
        switchToPartVIKnotMode();
        return;
    }

    if (button == &showBezierModeButton)
    {
        switchToBezierMode();
        return;
    }
}

std::vector<juce::Point<double>>& MainContentComponent::getActiveControlPoints()
{
    return usingPartVIKnots ? deBoorPoints : bezierPoints;
}

const std::vector<juce::Point<double>>& MainContentComponent::getActiveControlPoints() const
{
    return usingPartVIKnots ? deBoorPoints : bezierPoints;
}

int MainContentComponent::findControlPointNear(const PlotArea& plot, juce::Point<float> mousePos) const
{
    const auto& points = getActiveControlPoints();
    const float maxDist2 = (pointRadius + 4.0f) * (pointRadius + 4.0f);

    for (int i = 0; i < (int) points.size(); ++i)
    {
        const auto screen = worldToScreen(plot, points[(size_t) i]);
        const auto delta = screen - mousePos;
        if (delta.getDistanceSquaredFromOrigin() <= maxDist2)
            return i;
    }

    return -1;
}

void MainContentComponent::dragActiveControlPointTo(const PlotArea& plot, juce::Point<float> mousePos)
{
    auto& points = getActiveControlPoints();
    if (selectedPoint < 0 || selectedPoint >= (int) points.size())
        return;

    points[(size_t) selectedPoint] = screenToWorld(plot, mousePos);
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

juce::Point<double> MainContentComponent::evaluateCurrentCurve(double t) const
{
    if (usingPartVIKnots && !deBoorPoints.empty())
        return evaluateDeBoorCurve(deBoorPoints, currentKnots, t);

    return evaluateBezierByDeCasteljau(t);
}

juce::Point<double> MainContentComponent::evaluateDeBoorCurve(const std::vector<juce::Point<double>>& controlPoints,
                                                              const std::vector<double>& knots,
                                                              double t) const
{
    if (controlPoints.empty())
        return { 0.0, 0.0 };

    const int d = degree;
    const int s = (int) controlPoints.size() - 1;
    const int N = (int) knots.size() - 1;

    if (s != d || N != 2 * d + 1)
        return controlPoints.back();

    const double left = knots[(size_t) d];
    const double right = knots[(size_t) (N - d)];
    t = juce::jlimit(left, std::nextafter(right, left), t);

    int J = d;
    for (int j = d; j < N - d; ++j)
    {
        if (t >= knots[(size_t) j] && t < knots[(size_t) (j + 1)])
        {
            J = j;
            break;
        }
    }

    std::vector<juce::Point<double>> work = controlPoints;
    for (int p = 1; p <= d; ++p)
    {
        for (int i = J - d + p; i <= J; ++i)
        {
            const double leftKnot = knots[(size_t) i];
            const double rightKnot = knots[(size_t) (i + d - p + 1)];
            const double denom = rightKnot - leftKnot;

            double alpha = 0.0;
            if (std::abs(denom) > 1.0e-12)
                alpha = (t - leftKnot) / denom;

            work[(size_t) i] = lerpPoint(work[(size_t) (i - 1)], work[(size_t) i], alpha);
        }
    }

    return work[(size_t) J];
}

juce::Point<double> MainContentComponent::evaluateBezierByDeCasteljau(double t) const
{
    if (bezierPoints.empty())
        return { 0.0, 0.0 };

    std::vector<juce::Point<double>> work = bezierPoints;
    const int d = (int) work.size() - 1;

    for (int level = 1; level <= d; ++level)
    {
        for (int i = 0; i <= d - level; ++i)
            work[(size_t) i] = lerpPoint(work[(size_t) i], work[(size_t) (i + 1)], t);
    }

    return work.front();
}

juce::Point<double> MainContentComponent::evaluateBlossom(const std::vector<double>& u) const
{
    if ((int) bezierPoints.size() != degree + 1 || (int) u.size() != degree)
        return { 0.0, 0.0 };

    std::vector<juce::Point<double>> work = bezierPoints;

    for (int r = 1; r <= degree; ++r)
    {
        const double ur = u[(size_t) (r - 1)];
        for (int i = 0; i <= degree - r; ++i)
            work[(size_t) i] = lerpPoint(work[(size_t) i], work[(size_t) (i + 1)], ur);
    }

    return work.front();
}

double MainContentComponent::clamp01(double value)
{
    return juce::jlimit(0.0, 1.0, value);
}

juce::String MainContentComponent::knotSequenceToString(const std::vector<double>& knots)
{
    juce::StringArray parts;
    for (double knot : knots)
        parts.add(juce::String(knot, 2));
    return parts.joinIntoString(", ");
}
