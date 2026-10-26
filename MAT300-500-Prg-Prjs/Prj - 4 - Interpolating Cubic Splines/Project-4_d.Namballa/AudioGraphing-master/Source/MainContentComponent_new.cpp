#include "MainContentComponent_new.h"

static constexpr int kCurveSamples = 600;

//==============================================================================
MainContentComponent::MainContentComponent()
{
    setOpaque(true);
}

void MainContentComponent::resized() {}

//==============================================================================
void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto bounds = getLocalBounds();
    auto infoArea = bounds.removeFromTop(infoPanelHeight);
    auto canvasArea = bounds;

    drawInfoPanel(g, infoArea);

    g.saveState();
    g.reduceClipRegion(canvasArea);

    // Draw spline curve
    if ((int)points.size() >= 2 && splineX.size() == (int)points.size())
    {
        g.setColour(juce::Colours::deepskyblue);
        g.strokePath(buildCurvePath(), juce::PathStrokeType(2.5f));
    }

    // Draw control points
    for (int i = 0; i < (int)points.size(); ++i)
    {
        const auto p = points[(size_t)i];

        g.setColour(i == selectedIndex ? juce::Colours::yellow : juce::Colours::orange);
        g.fillEllipse(p.x - pointRadius, p.y - pointRadius,
            2.0f * pointRadius, 2.0f * pointRadius);

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText("P" + juce::String(i),
            juce::Rectangle<float>(p.x + 10.0f, p.y - 14.0f, 60.0f, 20.0f),
            juce::Justification::centredLeft);
    }

    g.restoreState();

    // Bottom hint
    g.setColour(juce::Colours::grey);
    g.setFont(13.0f);
    g.drawText("Click to add points. Drag existing points to move them.",
        canvasArea.removeFromBottom(22),
        juce::Justification::centredLeft);
}

//==============================================================================
void MainContentComponent::drawInfoPanel(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(juce::Colours::azure);
    g.fillRect(area);
    g.setColour(juce::Colour(0xff90c0d0));
    g.drawRect(area, 1);

    auto inner = area.reduced(10);

    const int n = (int)points.size();
    const int k = (n > 0) ? n - 1 : 0;   // number of intervals / last knot
    const int sys = n + 2;                  // system dimension

    g.setColour(juce::Colours::black);
    g.setFont(16.0f);
    g.drawText("MATH 500 - Project 4: Natural Cubic Spline Interpolation (Truncated Power Basis)",
        inner.removeFromTop(22), juce::Justification::centredLeft);

    g.setFont(14.0f);
    g.drawText("Points n = " + juce::String(n)
        + "    Intervals k = " + juce::String(k)
        + "    System: (" + juce::String(sys) + " x " + juce::String(sys) + ")"
        + "    Unknowns: [a0, a1, a2, a3"
        + (k > 1 ? ", b1 .. b" + juce::String(k - 1) : "")
        + "]    BCs: f''(0) = f''(k) = 0",
        inner.removeFromTop(20), juce::Justification::centredLeft);

    g.setFont(13.0f);

    if (n < 2)
    {
        g.drawText("Add at least 2 points to compute the spline.",
            inner, juce::Justification::centredLeft);
        return;
    }

    // Show coefficients for x(t) and y(t) (up to 8 each)
    const auto& cx = splineX.getCoeffs();
    const auto& cy = splineY.getCoeffs();

    auto coeffLabel = [](int i) -> juce::String
        {
            if (i == 0) return "a0";
            if (i == 1) return "a1";
            if (i == 2) return "a2";
            if (i == 3) return "a3";
            return "b" + juce::String(i - 3);
        };

    const int maxShow = juce::jmin((int)cx.size(), 8);
    juce::String lineX("x(t): "), lineY("y(t): ");
    for (int i = 0; i < maxShow; ++i)
    {
        lineX += coeffLabel(i) + "=" + juce::String(cx[(size_t)i], 3) + "  ";
        lineY += coeffLabel(i) + "=" + juce::String(cy[(size_t)i], 3) + "  ";
    }
    if ((int)cx.size() > maxShow) { lineX += "..."; lineY += "..."; }

    g.drawText(lineX, inner.removeFromTop(18), juce::Justification::centredLeft);
    g.drawText(lineY, inner.removeFromTop(18), juce::Justification::centredLeft);
}

//==============================================================================
int MainContentComponent::findPointNear(juce::Point<float> p, float radius) const
{
    const float r2 = radius * radius;
    for (int i = 0; i < (int)points.size(); ++i)
    {
        const auto d = points[(size_t)i] - p;
        if (d.getDistanceSquaredFromOrigin() <= r2)
            return i;
    }
    return -1;
}

//==============================================================================
void MainContentComponent::mouseDown(const juce::MouseEvent& e)
{
    const auto pos = e.position;
    if (pos.y < (float)infoPanelHeight) return;

    selectedIndex = findPointNear(pos, pickRadius);

    if (selectedIndex >= 0)
    {
        dragging = true;
        repaint();
        return;
    }

    // Empty space: add new point
    points.push_back(pos);
    selectedIndex = (int)points.size() - 1;
    dragging = true;
    rebuildSplines();
    repaint();
}

void MainContentComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragging || selectedIndex < 0) return;

    auto p = e.position;
    if (p.y < (float)infoPanelHeight) p.y = (float)infoPanelHeight;

    points[(size_t)selectedIndex] = p;
    rebuildSplines();
    repaint();
}

void MainContentComponent::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
    selectedIndex = -1;
}

//==============================================================================
void MainContentComponent::rebuildSplines()
{
    const int n = (int)points.size();
    if (n == 0) { splineX.clear(); splineY.clear(); return; }

    std::vector<double> tNodes, xVals, yVals;
    tNodes.reserve((size_t)n);
    xVals.reserve((size_t)n);
    yVals.reserve((size_t)n);

    for (int i = 0; i < n; ++i)
    {
        tNodes.push_back((double)i);
        xVals.push_back((double)points[(size_t)i].x);
        yVals.push_back((double)points[(size_t)i].y);
    }

    splineX.setData(tNodes, xVals);
    splineY.setData(tNodes, yVals);
}

//==============================================================================
juce::Path MainContentComponent::buildCurvePath() const
{
    juce::Path path;

    const int n = (int)points.size();
    if (n < 2 || splineX.size() != n) return path;

    const double tMax = (double)(n - 1);

    for (int s = 0; s < kCurveSamples; ++s)
    {
        const double t = tMax * (double)s / (double)(kCurveSamples - 1);
        const float  x = (float)splineX.evaluate(t);
        const float  y = (float)splineY.evaluate(t);

        if (s == 0) path.startNewSubPath(x, y);
        else        path.lineTo(x, y);
    }

    return path;
}
