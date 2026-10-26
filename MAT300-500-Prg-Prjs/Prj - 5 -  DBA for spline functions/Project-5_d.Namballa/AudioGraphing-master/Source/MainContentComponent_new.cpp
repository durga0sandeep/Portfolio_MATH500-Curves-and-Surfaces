#include "MainContentComponent_new.h"

#include <algorithm>
#include <cmath>

MainContentComponent::MainContentComponent()
{
    setOpaque(true);
    initialiseUi();
    syncCoefficientCount(false);
}

void MainContentComponent::initialiseUi()
{
    degreeLabel.setText("Degree", juce::dontSendNotification);
    addAndMakeVisible(degreeLabel);

    degreeSlider.setRange(1, 10, 1);
    degreeSlider.setValue(degree);
    degreeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    degreeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    degreeSlider.addListener(this);
    addAndMakeVisible(degreeSlider);

    intervalLabel.setText("Intervals N", juce::dontSendNotification);
    addAndMakeVisible(intervalLabel);

    intervalSlider.setRange(3, 24, 1);
    intervalSlider.setValue(intervalsN);
    intervalSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    intervalSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    intervalSlider.addListener(this);
    addAndMakeVisible(intervalSlider);

    methodLabel.setText("Method", juce::dontSendNotification);
    addAndMakeVisible(methodLabel);

    methodCombo.addItem("De Boor", (int) EvaluationMethod::deBoor);
    methodCombo.addItem("Divided Differences", (int) EvaluationMethod::dividedDifferences);
    methodCombo.addItem("Shifted Power Functions", (int) EvaluationMethod::shiftedPowerFunctions);
    methodCombo.setSelectedId((int) currentMethod);
    methodCombo.addListener(this);
    addAndMakeVisible(methodCombo);

    resetButton.addListener(this);
    addAndMakeVisible(resetButton);
}

void MainContentComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto top = area.removeFromTop(topPanelHeight - 10);

    auto row1 = top.removeFromTop(28);
    degreeLabel.setBounds(row1.removeFromLeft(55));
    degreeSlider.setBounds(row1.removeFromLeft(250));
    row1.removeFromLeft(10);
    intervalLabel.setBounds(row1.removeFromLeft(70));
    intervalSlider.setBounds(row1.removeFromLeft(260));

    auto row2 = top.removeFromTop(30);
    methodLabel.setBounds(row2.removeFromLeft(55));
    methodCombo.setBounds(row2.removeFromLeft(250));
    row2.removeFromLeft(20);
    resetButton.setBounds(row2.removeFromLeft(170));
}

void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto bounds = getLocalBounds().reduced(10);
    auto controlArea = bounds.removeFromTop(topPanelHeight);
    auto plotArea = bounds.removeFromTop(bounds.getHeight() - 30);
    auto statusArea = bounds;

    drawControls(g, controlArea);
    drawPlot(g, plotArea);
    drawStatusText(g, statusArea);
}

void MainContentComponent::drawControls(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour(0xff20232b));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff4a5568));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);

    auto title = area.reduced(10).removeFromTop(20);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    /*
    g.drawText("MATH 500 - Part V: De Boor Algorithm for Spline Functions", title,
               juce::Justification::centredLeft);
               */
}

MainContentComponent::PlotArea MainContentComponent::makePlotArea(juce::Rectangle<int> area) const
{
    PlotArea plot;
    plot.rect = area.toFloat().reduced(6.0f);
    plot.xMin = 0.0;
    plot.xMax = (double) intervalsN;
    plot.yMin = -3.0;
    plot.yMax = 3.0;
    return plot;
}

juce::Point<float> MainContentComponent::worldToScreen(const PlotArea& plot, double x, double y) const
{
    const auto px = (float) juce::jmap(x, plot.xMin, plot.xMax,
                                       (double) plot.rect.getX(),
                                       (double) plot.rect.getRight());
    const auto py = (float) juce::jmap(y, plot.yMin, plot.yMax,
                                       (double) plot.rect.getBottom(),
                                       (double) plot.rect.getY());
    return { px, py };
}

juce::Point<double> MainContentComponent::screenToWorld(const PlotArea& plot, juce::Point<float> p) const
{
    const auto x = juce::jmap((double) p.x,
                              (double) plot.rect.getX(), (double) plot.rect.getRight(),
                              plot.xMin, plot.xMax);
    const auto y = juce::jmap((double) p.y,
                              (double) plot.rect.getBottom(), (double) plot.rect.getY(),
                              plot.yMin, plot.yMax);
    return { x, y };
}

void MainContentComponent::drawPlot(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto plot = makePlotArea(area);

    g.setColour(juce::Colour(0xff0f1115));
    g.fillRoundedRectangle(plot.rect, 6.0f);
    g.setColour(juce::Colour(0xff3f4854));
    g.drawRoundedRectangle(plot.rect, 6.0f, 1.0f);

    drawAxes(g, plot);
    drawCurve(g, plot);
    drawCoefficientPolyline(g, plot);
    drawCoefficientDots(g, plot);
}

void MainContentComponent::drawAxes(juce::Graphics& g, const PlotArea& plot)
{
    g.setFont(12.0f);

    g.setColour(juce::Colour(0xff26303a));
    for (int x = 0; x <= intervalsN; ++x)
    {
        auto p0 = worldToScreen(plot, (double) x, plot.yMin);
        auto p1 = worldToScreen(plot, (double) x, plot.yMax);
        g.drawLine({ p0.x, p0.y, p1.x, p1.y }, 0.7f);
    }
    for (int y = -3; y <= 3; ++y)
    {
        auto p0 = worldToScreen(plot, plot.xMin, (double) y);
        auto p1 = worldToScreen(plot, plot.xMax, (double) y);
        g.drawLine({ p0.x, p0.y, p1.x, p1.y }, 0.7f);
    }

    g.setColour(juce::Colour(0xff7f8ea3));
    auto x0 = worldToScreen(plot, plot.xMin, 0.0);
    auto x1 = worldToScreen(plot, plot.xMax, 0.0);
    g.drawLine({ x0.x, x0.y, x1.x, x1.y }, 1.2f);

    for (int x = 0; x <= intervalsN; ++x)
    {
        auto p = worldToScreen(plot, (double) x, 0.0);
        g.setColour(juce::Colours::lightgrey);
        g.drawText(juce::String(x), juce::Rectangle<int>((int) p.x - 10, (int) plot.rect.getBottom() - 18, 24, 16),
                   juce::Justification::centred);
    }

    for (int y = -3; y <= 3; ++y)
    {
        auto p = worldToScreen(plot, 0.0, (double) y);
        g.setColour(juce::Colours::lightgrey);
        g.drawText(juce::String(y), juce::Rectangle<int>((int) plot.rect.getX() + 4, (int) p.y - 8, 26, 16),
                   juce::Justification::centredLeft);
    }
}

void MainContentComponent::drawCurve(juce::Graphics& g, const PlotArea& plot)
{
    if (coefficients.empty())
        return;

    const double tMin = (double) degree;
    const double tMax = (double) (intervalsN - degree);
    if (tMax <= tMin)
        return;

    juce::Path path;
    for (int s = 0; s < curveSamples; ++s)
    {
        double t = tMin + (tMax - tMin) * (double) s / (double) (curveSamples - 1);
        if (s == curveSamples - 1)
            t = std::nextafter(tMax, tMin);

        const double y = evaluateSpline(t);
        auto pt = worldToScreen(plot, t, y);
        if (s == 0) path.startNewSubPath(pt);
        else path.lineTo(pt);
    }

    g.setColour(juce::Colours::deepskyblue);
    g.strokePath(path, juce::PathStrokeType(2.5f));
}

void MainContentComponent::drawCoefficientPolyline(juce::Graphics& g, const PlotArea& plot)
{
    if (coefficients.empty())
        return;

    juce::Path path;
    for (int i = 0; i < (int) coefficients.size(); ++i)
    {
        auto p = worldToScreen(plot, getCoefficientCenterX(i), coefficients[(size_t) i]);
        if (i == 0) path.startNewSubPath(p);
        else path.lineTo(p);
    }

    g.setColour(juce::Colour(0x88ffcc66));
    g.strokePath(path, juce::PathStrokeType(1.2f));
}

void MainContentComponent::drawCoefficientDots(juce::Graphics& g, const PlotArea& plot)
{
    for (int i = 0; i < (int) coefficients.size(); ++i)
    {
        const auto center = worldToScreen(plot, getCoefficientCenterX(i), coefficients[(size_t) i]);
        g.setColour(i == selectedCoeff ? juce::Colours::yellow : juce::Colours::orange);
        g.fillEllipse(center.x - pointRadius, center.y - pointRadius,
                      2.0f * pointRadius, 2.0f * pointRadius);

        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("c" + juce::String(i),
                   juce::Rectangle<float>(center.x + 8.0f, center.y - 10.0f, 32.0f, 16.0f),
                   juce::Justification::centredLeft);
    }
}

void MainContentComponent::drawStatusText(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colours::grey);
    g.setFont(13.0f);

    const auto coeffCount = getCoefficientCount();
    const juce::String methodName = methodCombo.getText();
    g.drawText("Coefficient count = N - d = " + juce::String(coeffCount)
               + "    Domain: [" + juce::String(degree) + ", " + juce::String(intervalsN - degree) + "]"
               + "    Method: " + methodName
               + "    Drag dots vertically to change coefficients.",
               area, juce::Justification::centredLeft);
}

int MainContentComponent::getCoefficientCount() const
{
    return juce::jmax(1, intervalsN - degree);
}

double MainContentComponent::getDefaultIntervalCountForDegree(int d) const
{
    return (double) (d + 10);
}

void MainContentComponent::resetCoefficients()
{
    coefficients.assign((size_t) getCoefficientCount(), 1.0);
}

void MainContentComponent::syncCoefficientCount(bool preserveExisting)
{
    const int newCount = getCoefficientCount();
    std::vector<double> updated((size_t) newCount, 1.0);

    if (preserveExisting)
    {
        const int copyCount = juce::jmin((int) coefficients.size(), newCount);
        for (int i = 0; i < copyCount; ++i)
            updated[(size_t) i] = coefficients[(size_t) i];
    }

    coefficients = std::move(updated);
    selectedCoeff = juce::jlimit(-1, newCount - 1, selectedCoeff);
}

void MainContentComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &degreeSlider)
    {
        degree = (int) degreeSlider.getValue();

        const int minN = degree + 1;
        intervalSlider.setRange(minN, 30, 1);
        if (intervalsN < minN)
        {
            intervalsN = minN;
            intervalSlider.setValue(intervalsN, juce::dontSendNotification);
        }

        syncCoefficientCount(true);
        repaint();
        return;
    }

    if (slider == &intervalSlider)
    {
        intervalsN = (int) intervalSlider.getValue();
        syncCoefficientCount(true);
        repaint();
        return;
    }
}

void MainContentComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &methodCombo)
    {
        currentMethod = (EvaluationMethod) methodCombo.getSelectedId();
        repaint();
    }
}

void MainContentComponent::buttonClicked(juce::Button* button)
{
    if (button == &resetButton)
    {
        resetCoefficients();
        repaint();
    }
}

double MainContentComponent::getCoefficientCenterX(int index) const
{
    return ((double) degree + 1.0) / 2.0 + (double) index;
}

int MainContentComponent::findCoefficientNear(const PlotArea& plot, juce::Point<float> p) const
{
    const float r2 = (pointRadius + 4.0f) * (pointRadius + 4.0f);
    for (int i = 0; i < (int) coefficients.size(); ++i)
    {
        auto c = worldToScreen(plot, getCoefficientCenterX(i), coefficients[(size_t) i]);
        auto d = c - p;
        if (d.getDistanceSquaredFromOrigin() <= r2)
            return i;
    }
    return -1;
}

void MainContentComponent::clampSelectedCoefficientY(const PlotArea& plot, juce::Point<float> mousePos)
{
    if (selectedCoeff < 0 || selectedCoeff >= (int) coefficients.size())
        return;

    const auto world = screenToWorld(plot, mousePos);
    coefficients[(size_t) selectedCoeff] = juce::jlimit(plot.yMin, plot.yMax, world.y);
}

void MainContentComponent::mouseDown(const juce::MouseEvent& e)
{
    auto plot = makePlotArea(getLocalBounds().reduced(10).withTrimmedTop(topPanelHeight).withTrimmedBottom(30));
    if (!plot.rect.contains(e.position))
        return;

    selectedCoeff = findCoefficientNear(plot, e.position);
    draggingCoeff = (selectedCoeff >= 0);
    repaint();
}

void MainContentComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!draggingCoeff)
        return;

    auto plot = makePlotArea(getLocalBounds().reduced(10).withTrimmedTop(topPanelHeight).withTrimmedBottom(30));
    clampSelectedCoefficientY(plot, e.position);
    repaint();
}

void MainContentComponent::mouseUp(const juce::MouseEvent&)
{
    draggingCoeff = false;
    selectedCoeff = -1;
    repaint();
}

double MainContentComponent::evaluateSpline(double t) const
{
    switch (currentMethod)
    {
        case EvaluationMethod::deBoor: return evaluateByDeBoor(t);
        case EvaluationMethod::dividedDifferences: return evaluateByDividedDifferences(t);
        case EvaluationMethod::shiftedPowerFunctions: return evaluateByShiftedPowers(t);
        default: return evaluateByDeBoor(t);
    }
}

double MainContentComponent::evaluateByDeBoor(double t) const
{
    if (coefficients.empty())
        return 0.0;

    const int d = degree;
    const int m = (int) coefficients.size(); // m = N - d
    const int N = intervalsN;

    if (m <= 0)
        return 0.0;

    const double left = (double) d;
    const double right = (double) (N - d);
    if (t <= left) t = left;
    if (t >= right) t = std::nextafter(right, left);

    int J = (int) std::floor(t);
    J = juce::jlimit(d, N - d - 1, J);

    std::vector<double> work = coefficients;
    for (int p = 1; p <= d; ++p)
    {
        for (int i = J - d + p; i <= J; ++i)
        {
            if (i < 0 || i >= m)
                continue;

            const double denom = (double) (d - p + 1);
            const double alpha = ((t - (double) i) / denom);
            const double beta = (((double) i + denom - t) / denom);
            const double cim1 = (i - 1 >= 0 && i - 1 < m) ? work[(size_t) (i - 1)] : 0.0;
            const double ci = work[(size_t) i];
            work[(size_t) i] = alpha * ci + beta * cim1;
        }
    }

    return work[(size_t) J];
}

double MainContentComponent::shiftedPower(double x, int d)
{
    if (x <= 0.0)
        return 0.0;
    return std::pow(x, (double) d);
}

double MainContentComponent::factorial(int n)
{
    double f = 1.0;
    for (int i = 2; i <= n; ++i)
        f *= (double) i;
    return f;
}

double MainContentComponent::binomial(int n, int k)
{
    if (k < 0 || k > n)
        return 0.0;
    if (k == 0 || k == n)
        return 1.0;
    k = juce::jmin(k, n - k);
    double result = 1.0;
    for (int i = 1; i <= k; ++i)
        result = result * (double) (n - k + i) / (double) i;
    return result;
}

double MainContentComponent::evaluateBasisShiftedPower(int i, int d, double t) const
{
    double sum = 0.0;
    for (int j = 0; j <= d + 1; ++j)
    {
        const double term = binomial(d + 1, j) * shiftedPower(t - (double) (i + j), d);
        sum += ((j % 2 == 0) ? 1.0 : -1.0) * term;
    }
    return sum / factorial(d);
}

double MainContentComponent::evaluateByShiftedPowers(double t) const
{
    double value = 0.0;
    for (int i = 0; i < (int) coefficients.size(); ++i)
        value += coefficients[(size_t) i] * evaluateBasisShiftedPower(i, degree, t);
    return value;
}

double MainContentComponent::recursiveDividedDifference(const std::vector<double>& xs,
                                                        int left,
                                                        int right,
                                                        const std::function<double(double)>& f)
{
    if (left == right)
        return f(xs[(size_t) left]);

    const double denom = xs[(size_t) right] - xs[(size_t) left];
    if (std::abs(denom) < 1e-14)
        return 0.0;

    const double upper = recursiveDividedDifference(xs, left + 1, right, f);
    const double lower = recursiveDividedDifference(xs, left, right - 1, f);
    return (upper - lower) / denom;
}

double MainContentComponent::evaluateBasisDividedDifference(int i, int d, double t) const
{
    std::vector<double> xs;
    xs.reserve((size_t) (d + 2));
    for (int k = 0; k <= d + 1; ++k)
        xs.push_back((double) (i + k));

    auto f = [t, d](double x) -> double
    {
        return shiftedPower(t - x, d);
    };

    const double dd = recursiveDividedDifference(xs, 0, d + 1, f);
    const double factor = (((d + 1) % 2 == 0) ? 1.0 : -1.0) * (double)(d + 1);
    return factor * dd;
}

double MainContentComponent::evaluateByDividedDifferences(double t) const
{
    double value = 0.0;
    for (int i = 0; i < (int) coefficients.size(); ++i)
        value += coefficients[(size_t) i] * evaluateBasisDividedDifference(i, degree, t);
    return value;
}
