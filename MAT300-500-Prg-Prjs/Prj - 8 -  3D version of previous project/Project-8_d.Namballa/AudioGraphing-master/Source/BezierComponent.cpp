#include "BezierComponent.h"
#include <cmath>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;
}

BezierComponent::BezierComponent()
{
    initializeDefaultControlPoints();
}

void BezierComponent::initializeDefaultControlPoints()
{
    controlPoints.clear();
    controlPoints.reserve(MAX_CONTROL_POINTS);

    controlPoints.push_back({ -2.2f, -1.1f, -0.9f });
    controlPoints.push_back({ -0.9f,  1.6f,  1.2f });
    controlPoints.push_back({  1.0f, -1.5f, -1.0f });
    controlPoints.push_back({  2.3f,  1.0f,  0.8f });

    selectedPointIndex = -1;
    markCurveDirty();
}

void BezierComponent::resized()
{
}

void BezierComponent::markCurveDirty()
{
    curveDirty = true;
    shellsDirty = true;
}

void BezierComponent::markShellsDirty()
{
    shellsDirty = true;
}

BezierComponent::Point3D BezierComponent::lerp(const Point3D& a, const Point3D& b, float t) const
{
    const float omt = 1.0f - t;
    return {
        omt * a.x + t * b.x,
        omt * a.y + t * b.y,
        omt * a.z + t * b.z
    };
}

juce::Point<float> BezierComponent::projectPoint(const Point3D& p) const
{
    const float cy = std::cos(yaw);
    const float sy = std::sin(yaw);
    const float cp = std::cos(pitch);
    const float sp = std::sin(pitch);

    const float x1 = cy * p.x + sy * p.z;
    const float z1 = -sy * p.x + cy * p.z;

    const float y2 = cp * p.y - sp * z1;
    const float z2 = sp * p.y + cp * z1;

    const float denom = juce::jmax(1.0f, z2 + cameraDistance);
    const float f = (sceneScale * zoom) / denom;

    const float screenX = static_cast<float>(getWidth()) * 0.5f + x1 * f;
    const float screenY = static_cast<float>(getHeight()) * 0.56f - y2 * f;
    return { screenX, screenY };
}

BezierComponent::Point3D BezierComponent::screenToWorld(juce::Point<float> screenPoint, float zPlane) const
{
    const float cy = std::cos(yaw);
    const float sy = std::sin(yaw);
    const float cp = std::cos(pitch);
    const float sp = std::sin(pitch);

    const float cx = static_cast<float>(getWidth()) * 0.5f;
    const float cyScreen = static_cast<float>(getHeight()) * 0.56f;

    // Approximate current perspective scale using selected z plane.
    const float rotatedZPlane = cp * (cy * zPlane) + cameraDistance;
    const float denom = juce::jmax(1.0f, rotatedZPlane);
    const float f = (sceneScale * zoom) / denom;

    const float x1 = (screenPoint.x - cx) / f;
    const float y2 = (cyScreen - screenPoint.y) / f;

    // Inverse rotation with z locked approximately to zPlane.
    const float z1 = (cp != 0.0f) ? ((-y2 * sp) + (zPlane * cp)) : zPlane;
    const float y = cp * y2 + sp * z1;
    const float x = cy * x1 - sy * z1;
    const float z = sy * x1 + cy * z1;

    return { x, y, zPlane + (z - zPlane) * 0.15f };
}

BezierComponent::Point3D BezierComponent::deCasteljauIterative(const std::vector<Point3D>& points, float t) const
{
    if (points.empty())
        return {};

    std::vector<Point3D> work = points;
    while (work.size() > 1)
    {
        std::vector<Point3D> next;
        next.reserve(work.size() - 1);
        for (size_t i = 0; i + 1 < work.size(); ++i)
            next.push_back(lerp(work[i], work[i + 1], t));
        work = std::move(next);
    }

    return work.front();
}

std::vector<std::vector<BezierComponent::Point3D>> BezierComponent::deCasteljauWithStages(const std::vector<Point3D>& points, float t) const
{
    std::vector<std::vector<Point3D>> stages;
    if (points.empty())
        return stages;

    stages.push_back(points);
    std::vector<Point3D> current = points;

    while (current.size() > 1)
    {
        std::vector<Point3D> next;
        next.reserve(current.size() - 1);
        for (size_t i = 0; i + 1 < current.size(); ++i)
            next.push_back(lerp(current[i], current[i + 1], t));
        stages.push_back(next);
        current = std::move(next);
    }

    return stages;
}

float BezierComponent::binomialCoefficient(int n, int k) const
{
    if (k < 0 || k > n)
        return 0.0f;
    if (k == 0 || k == n)
        return 1.0f;

    k = juce::jmin(k, n - k);
    double value = 1.0;
    for (int i = 1; i <= k; ++i)
        value = value * (n - (k - i)) / i;
    return static_cast<float>(value);
}

BezierComponent::Point3D BezierComponent::evaluateBezierBernstein(const std::vector<Point3D>& points, float t) const
{
    const int n = static_cast<int>(points.size()) - 1;
    Point3D result{};
    const float omt = 1.0f - t;

    for (int i = 0; i <= n; ++i)
    {
        const float b = binomialCoefficient(n, i) * std::pow(omt, static_cast<float>(n - i)) * std::pow(t, static_cast<float>(i));
        result.x += b * points[(size_t)i].x;
        result.y += b * points[(size_t)i].y;
        result.z += b * points[(size_t)i].z;
    }

    return result;
}

void BezierComponent::subdivisionStep(const std::vector<Point3D>& controlPts,
                                      std::vector<Point3D>& leftPts,
                                      std::vector<Point3D>& rightPts) const
{
    std::vector<std::vector<Point3D>> stages;
    stages.push_back(controlPts);

    std::vector<Point3D> current = controlPts;
    while (current.size() > 1)
    {
        std::vector<Point3D> next;
        next.reserve(current.size() - 1);
        for (size_t i = 0; i + 1 < current.size(); ++i)
            next.push_back(lerp(current[i], current[i + 1], 0.5f));
        stages.push_back(next);
        current = std::move(next);
    }

    leftPts.clear();
    rightPts.clear();
    leftPts.reserve(stages.size());
    rightPts.reserve(stages.size());

    for (const auto& stage : stages)
        leftPts.push_back(stage.front());

    for (auto it = stages.rbegin(); it != stages.rend(); ++it)
        rightPts.push_back(it->back());
}

std::vector<BezierComponent::Point3D> BezierComponent::generateCurveBySubdivision(const std::vector<Point3D>& controlPts, int k) const
{
    std::vector<std::vector<Point3D>> segments;
    segments.push_back(controlPts);

    for (int level = 0; level < k; ++level)
    {
        std::vector<std::vector<Point3D>> nextSegments;
        nextSegments.reserve(segments.size() * 2);

        for (const auto& segment : segments)
        {
            std::vector<Point3D> leftPts, rightPts;
            subdivisionStep(segment, leftPts, rightPts);
            nextSegments.push_back(leftPts);
            nextSegments.push_back(rightPts);
        }

        segments = std::move(nextSegments);
    }

    std::vector<Point3D> curve;
    for (const auto& segment : segments)
        curve.push_back(segment.front());
    if (!segments.empty())
        curve.push_back(segments.back().back());

    return curve;
}

int BezierComponent::getOptimalSampleCount() const
{
    const int n = static_cast<int>(controlPoints.size());
    return juce::jlimit(60, 280, 60 + (n - 2) * 22);
}

void BezierComponent::updateCachedCurve()
{
    cachedCurvePoints.clear();
    cachedSubdivisionPoints.clear();

    if (controlPoints.size() < 2)
        return;

    if (currentMethod == MIDPOINT_SUBDIVISION)
    {
        cachedSubdivisionPoints = generateCurveBySubdivision(controlPoints, subdivisionLevel);
        return;
    }

    const int steps = getOptimalSampleCount();
    cachedCurvePoints.reserve(static_cast<size_t>(steps + 1));

    for (int i = 0; i <= steps; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        cachedCurvePoints.push_back(currentMethod == NESTED_LINEAR_INTERPOLATION
                                        ? deCasteljauIterative(controlPoints, t)
                                        : evaluateBezierBernstein(controlPoints, t));
    }
}

void BezierComponent::updateCachedShells()
{
    cachedShellStages.clear();
    if (showShells && currentMethod == NESTED_LINEAR_INTERPOLATION && controlPoints.size() >= 2)
        cachedShellStages = deCasteljauWithStages(controlPoints, tValue);
}

void BezierComponent::drawAxes(juce::Graphics& g)
{
    const Point3D origin{ 0.0f, 0.0f, 0.0f };
    drawLine(g, { -3.5f, 0.0f, 0.0f }, { 3.5f, 0.0f, 0.0f }, juce::Colours::darkred.withAlpha(0.7f), 1.5f);
    drawLine(g, { 0.0f, -3.5f, 0.0f }, { 0.0f, 3.5f, 0.0f }, juce::Colours::darkgreen.withAlpha(0.7f), 1.5f);
    drawLine(g, { 0.0f, 0.0f, -3.5f }, { 0.0f, 0.0f, 3.5f }, juce::Colours::darkorange.withAlpha(0.7f), 1.5f);

    drawPoint(g, origin, juce::Colours::black, 4.0f);
    drawPointLabel(g, projectPoint({ 3.6f, 0.0f, 0.0f }), "X", juce::Colours::darkred);
    drawPointLabel(g, projectPoint({ 0.0f, 3.6f, 0.0f }), "Y", juce::Colours::darkgreen);
    drawPointLabel(g, projectPoint({ 0.0f, 0.0f, 3.6f }), "Z", juce::Colours::darkorange.darker());
}

void BezierComponent::drawBezierCurve(juce::Graphics& g)
{
    const auto& pts = (currentMethod == MIDPOINT_SUBDIVISION) ? cachedSubdivisionPoints : cachedCurvePoints;
    if (pts.size() < 2)
        return;

    g.setColour(juce::Colours::blue);
    for (size_t i = 1; i < pts.size(); ++i)
    {
        const auto p1 = projectPoint(pts[i - 1]);
        const auto p2 = projectPoint(pts[i]);
        g.drawLine(p1.x, p1.y, p2.x, p2.y, 2.5f);
    }

    if (currentMethod != MIDPOINT_SUBDIVISION)
        drawPoint(g, deCasteljauIterative(controlPoints, tValue), juce::Colours::red, 8.0f);
}

void BezierComponent::drawControlPolyline(juce::Graphics& g)
{
    if (controlPoints.size() < 2)
        return;

    for (size_t i = 1; i < controlPoints.size(); ++i)
        drawLine(g, controlPoints[i - 1], controlPoints[i], juce::Colours::grey, 1.2f);
}

void BezierComponent::drawControlPoints(juce::Graphics& g)
{
    for (size_t i = 0; i < controlPoints.size(); ++i)
    {
        const bool isSelected = static_cast<int>(i) == selectedPointIndex;
        drawPoint(g, controlPoints[i], isSelected ? juce::Colours::red : juce::Colours::darkblue, isSelected ? 10.0f : 8.0f);

        const auto p = projectPoint(controlPoints[i]);
        const auto& cp = controlPoints[i];
        const juce::String label = "P" + juce::String(static_cast<int>(i))
                                 + " (" + juce::String(cp.x, 1)
                                 + ", " + juce::String(cp.y, 1)
                                 + ", " + juce::String(cp.z, 1) + ")";
        drawPointLabel(g, { p.x + 8.0f, p.y - 18.0f }, label, juce::Colours::black);
    }
}

void BezierComponent::drawShells(juce::Graphics& g)
{
    if (cachedShellStages.empty())
        return;

    static const juce::Colour colors[] = {
        juce::Colours::lightgrey,
        juce::Colours::lightgreen,
        juce::Colours::lightcoral,
        juce::Colours::lightcyan,
        juce::Colours::khaki,
        juce::Colours::plum
    };

    for (size_t stageIdx = 1; stageIdx < cachedShellStages.size(); ++stageIdx)
    {
        const auto colour = colors[stageIdx % (sizeof(colors) / sizeof(colors[0]))];
        const auto& stage = cachedShellStages[stageIdx];

        for (size_t i = 0; i < stage.size(); ++i)
        {
            if (i + 1 < stage.size())
                drawLine(g, stage[i], stage[i + 1], colour, 1.8f);
            drawPoint(g, stage[i], colour.darker(0.35f), 5.0f);
        }
    }

    drawPoint(g, cachedShellStages.back().front(), juce::Colours::red, 8.0f);
}

void BezierComponent::drawPointLabel(juce::Graphics& g, const juce::Point<float>& p, const juce::String& text, juce::Colour colour)
{
    g.setColour(colour);
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    g.drawText(text, juce::Rectangle<float>(p.x, p.y, 180.0f, 18.0f), juce::Justification::left, false);
}

void BezierComponent::drawPoint(juce::Graphics& g, const Point3D& p, juce::Colour color, float radius)
{
    const auto sp = projectPoint(p);
    g.setColour(color);
    g.fillEllipse(sp.x - radius * 0.5f, sp.y - radius * 0.5f, radius, radius);
    g.setColour(color.darker());
    g.drawEllipse(sp.x - radius * 0.5f, sp.y - radius * 0.5f, radius, radius, 1.0f);
}

void BezierComponent::drawLine(juce::Graphics& g, const Point3D& p1, const Point3D& p2, juce::Colour color, float thickness)
{
    const auto a = projectPoint(p1);
    const auto b = projectPoint(p2);
    g.setColour(color);
    g.drawLine(a.x, a.y, b.x, b.y, thickness);
}

void BezierComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    g.setColour(juce::Colours::whitesmoke);
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(6.0f), 10.0f);

    g.setColour(juce::Colours::black.withAlpha(0.08f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(6.0f), 10.0f, 1.0f);

    if (curveDirty)
    {
        updateCachedCurve();
        curveDirty = false;
    }

    if (shellsDirty)
    {
        updateCachedShells();
        shellsDirty = false;
    }

    drawAxes(g);

    if (showControlPolyline)
        drawControlPolyline(g);

    drawBezierCurve(g);

    if (showShells && currentMethod == NESTED_LINEAR_INTERPOLATION)
        drawShells(g);

    drawControlPoints(g);

    g.setColour(juce::Colours::darkslategrey);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    juce::String methodText = "Method: ";
    if (currentMethod == NESTED_LINEAR_INTERPOLATION) methodText += "Nested Linear Interpolation";
    else if (currentMethod == BERNSTEIN_POLYNOMIALS) methodText += "Bernstein Basis";
    else methodText += "Midpoint Subdivision";

    const juce::String help = methodText
        + "   |   t = " + juce::String(tValue, 2)
        + "   |   Shift + Drag = move in Z";
    g.drawText(help, 16, getHeight() - 28, getWidth() - 32, 20, juce::Justification::left);
}

int BezierComponent::findClosestPoint(juce::Point<float> mousePos) const
{
    int closest = -1;
    float minDist = selectionRadius;

    for (size_t i = 0; i < controlPoints.size(); ++i)
    {
        const float dist = mousePos.getDistanceFrom(projectPoint(controlPoints[i]));
        if (dist < minDist)
        {
            minDist = dist;
            closest = static_cast<int>(i);
        }
    }

    return closest;
}

void BezierComponent::mouseDown(const juce::MouseEvent& event)
{
    lastDragPosition = event.position;
    selectedPointIndex = findClosestPoint(event.position);
    if (onSelectedPointChanged)
        onSelectedPointChanged(selectedPointIndex);
    repaint();
}

void BezierComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (selectedPointIndex < 0 || selectedPointIndex >= static_cast<int>(controlPoints.size()))
        return;

    auto& p = controlPoints[(size_t)selectedPointIndex];

    if (event.mods.isShiftDown())
    {
        const float deltaY = event.position.y - lastDragPosition.y;
        p.z -= deltaY * 0.015f / juce::jmax(zoom, 0.25f);
    }
    else
    {
        const Point3D oldWorld = screenToWorld(lastDragPosition, p.z);
        const Point3D newWorld = screenToWorld(event.position, p.z);
        p.x += (newWorld.x - oldWorld.x);
        p.y += (newWorld.y - oldWorld.y);
    }

    lastDragPosition = event.position;
    markCurveDirty();
    repaint();

    if (onSelectedPointChanged)
        onSelectedPointChanged(selectedPointIndex);
}

void BezierComponent::mouseUp(const juce::MouseEvent&)
{
}

void BezierComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (static_cast<int>(controlPoints.size()) >= MAX_CONTROL_POINTS)
        return;

    addControlPoint(event.position);
}

void BezierComponent::setComputationMethod(ComputationMethod method)
{
    if (currentMethod == method)
        return;

    currentMethod = method;
    markCurveDirty();
    repaint();
}

void BezierComponent::setSubdivisionLevel(int k)
{
    const int newLevel = juce::jlimit(1, 8, k);
    if (subdivisionLevel == newLevel)
        return;

    subdivisionLevel = newLevel;
    markCurveDirty();
    repaint();
}

void BezierComponent::setTValue(float t)
{
    const float newT = juce::jlimit(0.0f, 1.0f, t);
    if (std::abs(newT - tValue) < 1.0e-6f)
        return;

    tValue = newT;
    markShellsDirty();
    repaint();
}

void BezierComponent::toggleShowShells(bool show)
{
    showShells = show;
    markShellsDirty();
    repaint();
}

void BezierComponent::toggleShowControlPolyline(bool show)
{
    showControlPolyline = show;
    repaint();
}

void BezierComponent::setCameraYaw(float newYaw)
{
    yaw = newYaw;
    repaint();
}

void BezierComponent::setCameraPitch(float newPitch)
{
    pitch = juce::jlimit(-1.45f, 1.45f, newPitch);
    repaint();
}

void BezierComponent::setCameraZoom(float newZoom)
{
    zoom = juce::jlimit(0.35f, 3.5f, newZoom);
    repaint();
}

void BezierComponent::setSelectedPointZ(float zValue)
{
    if (selectedPointIndex < 0 || selectedPointIndex >= static_cast<int>(controlPoints.size()))
        return;

    controlPoints[(size_t)selectedPointIndex].z = zValue;
    markCurveDirty();
    repaint();
}

float BezierComponent::getSelectedPointZ() const
{
    if (selectedPointIndex < 0 || selectedPointIndex >= static_cast<int>(controlPoints.size()))
        return 0.0f;
    return controlPoints[(size_t)selectedPointIndex].z;
}

void BezierComponent::addControlPoint(Point3D point)
{
    if (static_cast<int>(controlPoints.size()) >= MAX_CONTROL_POINTS)
        return;

    controlPoints.push_back(point);
    selectedPointIndex = static_cast<int>(controlPoints.size()) - 1;
    markCurveDirty();
    repaint();

    if (onControlPointsChanged)
        onControlPointsChanged();
    if (onSelectedPointChanged)
        onSelectedPointChanged(selectedPointIndex);
}

void BezierComponent::addControlPoint(juce::Point<float> screenPoint)
{
    addControlPoint(screenToWorld(screenPoint, 0.0f));
}

void BezierComponent::removeLastControlPoint()
{
    if (static_cast<int>(controlPoints.size()) <= MIN_CONTROL_POINTS)
        return;

    controlPoints.pop_back();
    if (selectedPointIndex >= static_cast<int>(controlPoints.size()))
        selectedPointIndex = static_cast<int>(controlPoints.size()) - 1;

    markCurveDirty();
    repaint();

    if (onControlPointsChanged)
        onControlPointsChanged();
    if (onSelectedPointChanged)
        onSelectedPointChanged(selectedPointIndex);
}

void BezierComponent::clearControlPoints()
{
    initializeDefaultControlPoints();
    repaint();

    if (onControlPointsChanged)
        onControlPointsChanged();
    if (onSelectedPointChanged)
        onSelectedPointChanged(selectedPointIndex);
}
