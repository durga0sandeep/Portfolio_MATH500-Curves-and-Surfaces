#include "MainContentComponent_new.h"

static constexpr int kCurveSamples = 500;

/**
 * @brief Builds the full divided-difference table for Newton interpolation.
 *
 * @param t Vector of node values (t0, t1, ..., tn).
 * @param f Vector of function values at the nodes (f(t0), f(t1), ...).
 * @return std::vector<std::vector<double>> Full divided-difference table table[order][i].
 */
static std::vector<std::vector<double>> buildDividedDifferenceTable(
    const std::vector<double>& t,
    const std::vector<double>& f)
{
    const int n = (int)t.size();
    std::vector<std::vector<double>> table;
    if (n == 0) return table;

    table.resize((size_t)n);
    table[0] = f;

    for (int order = 1; order < n; ++order)
    {
        table[(size_t)order].resize((size_t)(n - order));
        for (int i = 0; i < n - order; ++i)
        {
            const double denom = t[(size_t)(i + order)] - t[(size_t)i];
            table[(size_t)order][(size_t)i] =
                (table[(size_t)(order - 1)][(size_t)(i + 1)] - table[(size_t)(order - 1)][(size_t)i]) / denom;
        }
    }
    return table;
}


/**
 * @brief Constructor for MainContentComponent.
 *        Sets up the component for drawing and interaction.
 */
MainContentComponent::MainContentComponent()
{
    setOpaque(true);
    setWantsKeyboardFocus(true);
}


/**
 * @brief Called when the component is resized.
 *        Can be used to layout subcomponents.
 */
void MainContentComponent::resized()
{
}

/**
 * @brief Polls whether a key is currently pressed.
 *        Specifically used to detect 'C' key for cut-mode dragging.
 *
 * @param isKeyDown Unused parameter.
 * @return true if key state changed, false otherwise.
 */
bool MainContentComponent::keyStateChanged(bool)
{
    // Poll whether 'C' is currently held down
    const bool nowDown = 
        juce::KeyPress::isKeyCurrentlyDown('c') ||
        juce::KeyPress::isKeyCurrentlyDown('C');

    if (cutKeyDown != nowDown)
    {
        cutKeyDown = nowDown;
        repaint();
    }

    return false;
}

/**
 * @brief Paints the component, including the curve, points, and info panel.
 *
 * @param g JUCE graphics context.
 */
void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto bounds = getLocalBounds();
    auto infoArea = bounds.removeFromTop(infoPanelHeight);
    auto canvasArea = bounds;

    drawInfoPanel(g, infoArea);

    g.saveState();
    g.reduceClipRegion(canvasArea);

    if (points.size() >= 2)
    {
        auto curve = buildCurvePath();
        g.setColour(juce::Colours::deepskyblue);
        g.strokePath(curve, juce::PathStrokeType(2.5f));
    }

    for (int i = 0; i < (int)points.size(); ++i)
    {
        const auto p = points[(size_t)i];

        // Highlight selected point in normal mode
        if (!hasTempPoint && i == selectedIndex)
            g.setColour(juce::Colours::yellow);
        else
            g.setColour(juce::Colours::orange);

        g.fillEllipse(p.x - pointRadius, p.y - pointRadius, 2.0f * pointRadius, 2.0f * pointRadius);

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText("P" + juce::String(i),
            juce::Rectangle<float>(p.x + 10.0f, p.y - 14.0f, 100.0f, 20.0f),
            juce::Justification::centredLeft);
    }

    // If cut-mode dragging, show the removed point as a floating "ghost" point
    
    
    if (hasTempPoint)
    {
        g.setColour(juce::Colours::yellow);
        g.fillEllipse(tempPoint.x - pointRadius, tempPoint.y - pointRadius,
            2.0f * pointRadius, 2.0f * pointRadius);

        /*
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText("CUT -> append last",
            juce::Rectangle<float>(tempPoint.x + 10.0f, tempPoint.y - 14.0f, 160.0f, 20.0f),
            juce::Justification::centredLeft);
        */
    }
    

    g.restoreState();

    g.setColour(juce::Colours::grey);
    g.setFont(13.0f);
    g.drawText("Normal: drag a point to move it. Hold 'C' while dragging to remove it (temporary) and append as LAST on release.",
        canvasArea.removeFromBottom(22),
        juce::Justification::centredLeft);
}


/**
 * @brief Draws the top info panel including points, degree, and Newton coefficients.
 *
 * @param g Graphics context.
 * @param area Rectangle area for the info panel.
 */
void MainContentComponent::drawInfoPanel(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(juce::Colours::azure);
    g.fillRect(area);
    g.setColour(juce::Colours::azure);
    g.drawRect(area, 1);

    auto inner = area.reduced(10);

    const int n = (int)points.size();
    const int degree = (n > 0 ? n - 1 : 0);

    g.setColour(juce::Colours::black);
    g.setFont(16.0f);
    g.drawText("Newton Interpolation (Parametric Polynomial)", inner.removeFromTop(22),
        juce::Justification::centredLeft);

    g.setFont(14.0f);
    g.drawText("Points: " + juce::String(n)
        + "    Degree: " + juce::String(degree)
        + (cutKeyDown ? "    [C held]" : "")
        + (hasTempPoint ? "    (Cut mode: point removed while dragging)" : ""),
        inner.removeFromTop(20),
        juce::Justification::centredLeft);

    g.setFont(13.0f);

    
    if (n == 0)
    {
        g.setColour(juce::Colours::black);
        g.drawText("Click in the canvas to add points.", inner, juce::Justification::centredLeft);
        return;
    }
    
    // Build tables for x and y to show Newton coefficients (first element of each order)
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

    /*
    const auto tableX = buildDividedDifferenceTable(tNodes, xVals);
    const auto tableY = buildDividedDifferenceTable(tNodes, yVals);

    const int maxShow = juce::jmin(n, 7);
    juce::String coeffLineX("x(t) coeffs: ");
    juce::String coeffLineY("y(t) coeffs: ");

    for (int k = 0; k < maxShow; ++k)
    {
        const double ax = (!tableX[(size_t)k].empty() ? tableX[(size_t)k][0] : 0.0);
        const double ay = (!tableY[(size_t)k].empty() ? tableY[(size_t)k][0] : 0.0);

        coeffLineX += "a" + juce::String(k) + "=" + juce::String(ax, 3);
        coeffLineY += "a" + juce::String(k) + "=" + juce::String(ay, 3);

        if (k != maxShow - 1) { coeffLineX += "   "; coeffLineY += "   "; }
    }
    if (n > maxShow) { coeffLineX += "   ..."; coeffLineY += "   ..."; }

    g.drawText(coeffLineX, inner.removeFromTop(18), juce::Justification::centredLeft);
    g.drawText(coeffLineY, inner.removeFromTop(18), juce::Justification::centredLeft);
    */
    g.setColour(juce::Colours::black);
    g.drawText("Drag: move points normally. Hold 'C' to cut the selected point from the table while dragging; on release it is appended as LAST (order changes).",
        inner, juce::Justification::centredLeft);
}



/**
 * @brief Finds a point near the given position within a given radius.
 *
 * @param p Position to check.
 * @param radius Radius threshold.
 * @return int Index of point found, -1 if none.
 */
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


/**
 * @brief Handles mouse down events: selects, drags, or adds points.
 *
 * @param e Mouse event info.
 */
void MainContentComponent::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.position;

    // ignore clicks in info panel
    if (pos.y < (float)infoPanelHeight)
        return;

    grabKeyboardFocus();

    selectedIndex = findPointNear(pos, pickRadius);

    if (selectedIndex >= 0)
    {
        dragging = true;

        // If C is held, switch to cut-drag behavior
        if (cutKeyDown)
        {
            beginCutDrag(selectedIndex);
            repaint();
            return;
        }

        // Normal drag: DO NOT REMOVE. We update position live in mouseDrag.
        repaint();
        return;
    }

    // Click empty space -> add new point at end
    points.push_back(pos);
    dragging = true;

    rebuildInterpolators();
    repaint();
}


/**
 * @brief Handles mouse drag events to move or cut points.
 *
 * @param e Mouse event info.
 */
void MainContentComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragging) return;

    auto p = e.position;
    if (p.y < (float)infoPanelHeight) p.y = (float)infoPanelHeight;

    if (hasTempPoint)
    {
        // Cut mode: point is removed from interpolation set; curve is based on remaining points only
        tempPoint = p;
        repaint();
        return;
    }

    // Normal drag mode: point stays inside the interpolation set, update continuously
    if (selectedIndex >= 0 && selectedIndex < (int)points.size())
    {
        points[(size_t)selectedIndex] = p;
        rebuildInterpolators();
        repaint();
    }
}

/**
 * @brief Handles mouse up events, finalizing drag or cut operations.
 *
 * @param e Mouse event info.
 */
void MainContentComponent::mouseUp(const juce::MouseEvent&)
{
    dragging = false;

    if (hasTempPoint)
    {
        // Cut mode release: append point as LAST and recompute
        commitCutDragAppendLast();
        repaint();
        return;
    }

    // Normal drag release: just finish selection
    selectedIndex = -1;
}

/**
 * @brief Begins a cut-drag operation on a selected point.
 *
 * @param index Index of the point to cut.
 */
void MainContentComponent::beginCutDrag(int index)
{
    if (index < 0 || index >= (int)points.size())
        return;

    // store removed point
    tempPoint = points[(size_t)index];
    hasTempPoint = true;

    // remove from set
    points.erase(points.begin() + index);

    // no selected index because that point is temporarily not in the set
    selectedIndex = -1;

    // recompute curve WITHOUT it
    rebuildInterpolators();
}


/**
 * @brief Appends a cut point as the last point and rebuilds interpolation.
 */
void MainContentComponent::commitCutDragAppendLast()
{
    if (!hasTempPoint) return;

    // Append as LAST (changes interpolation order!)
    points.push_back(tempPoint);

    hasTempPoint = false;

    rebuildInterpolators();
}

/**
 * @brief Recomputes the interpolators for x(t) and y(t) using current points.
 */
void MainContentComponent::rebuildInterpolators()
{
    const int n = (int)points.size();
    if (n == 0)
    {
        interpX.clear();
        interpY.clear();
        return;
    }

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

    interpX.setData(tNodes, xVals);
    interpY.setData(tNodes, yVals);
}

/**
 * @brief Builds a JUCE Path representing the interpolated curve.
 *
 * @return juce::Path Path of the interpolated curve.
 */
juce::Path MainContentComponent::buildCurvePath() const
{
    juce::Path path;

    const int n = (int)points.size();
    if (n < 2 || interpX.size() != n || interpY.size() != n)
        return path;

    const double tMin = 0.0;
    const double tMax = (double)(n - 1);

    for (int s = 0; s < kCurveSamples; ++s)
    {
        const double alpha = (double)s / (double)(kCurveSamples - 1);
        const double t = tMin + alpha * (tMax - tMin);

        const float x = (float)interpX.evaluate(t);
        const float y = (float)interpY.evaluate(t);

        if (s == 0) path.startNewSubPath(x, y);
        else        path.lineTo(x, y);
    }

    return path;
}