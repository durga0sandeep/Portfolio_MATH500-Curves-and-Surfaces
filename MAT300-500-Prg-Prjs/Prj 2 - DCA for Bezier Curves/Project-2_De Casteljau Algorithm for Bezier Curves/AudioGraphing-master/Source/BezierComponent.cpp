/*
  ==============================================================================

    BezierComponent.cpp
    Created for MATH 500 - Project 2
    
  ==============================================================================
*/

#include "BezierComponent.h"
#include <cmath>

BezierComponent::BezierComponent()
    : selectedPointIndex(-1),
      selectionRadius(10.0f),
      currentMethod(NESTED_LINEAR_INTERPOLATION),
      subdivisionLevel(3),
      tValue(0.5f),
      showShells(true),
      showControlPolyline(true),
      curveDirty(true),
      shellsDirty(true)
{
    initializeDefaultControlPoints();
    
    // Pre-allocate buffers for common use cases
    tempPointBuffer.reserve(MAX_CONTROL_POINTS);
    
    // Initialize binomial cache
    updateBinomialCache(MAX_CONTROL_POINTS);
}

BezierComponent::~BezierComponent()
{
}

void BezierComponent::initializeDefaultControlPoints()
{
    controlPoints.clear();
    controlPoints.reserve(MAX_CONTROL_POINTS);
    controlPoints.push_back(juce::Point<float>(100.0f, 300.0f));
    controlPoints.push_back(juce::Point<float>(200.0f, 100.0f));
    controlPoints.push_back(juce::Point<float>(400.0f, 100.0f));
    controlPoints.push_back(juce::Point<float>(500.0f, 300.0f));
    markCurveDirty();
}

void BezierComponent::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colours::white);
    
    if (controlPoints.size() < 2)
        return;
    
    // OPTIMIZATION: Only update cached paths when necessary
    if (curveDirty)
    {
        updateCachedCurvePath();
        updateCachedPolylinePath();
        curveDirty = false;
    }
    
    if (shellsDirty && showShells && currentMethod == NESTED_LINEAR_INTERPOLATION)
    {
        updateCachedShells();
        shellsDirty = false;
    }
    
    // Draw Bezier curve (from cache)
    drawBezierCurve(g);
    
    // Draw shells
    if (showShells && currentMethod == NESTED_LINEAR_INTERPOLATION)
    {
        drawShells(g);
    }
    
    // Draw control polyline (from cache)
    if (showControlPolyline)
    {
        drawControlPolyline(g);
    }
    
    // Draw control points (always drawn fresh for immediate visual feedback)
    drawControlPoints(g);
}

void BezierComponent::resized()
{
}

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

void BezierComponent::markCurveDirty()
{
    curveDirty = true;
    shellsDirty = true;
}

void BezierComponent::markShellsDirty()
{
    shellsDirty = true;
}

// ============================================================================
// ADAPTIVE SAMPLING
// ============================================================================

int BezierComponent::getOptimalSampleCount() const
{
    // Base samples on number of control points and estimated complexity
    int n = (int)controlPoints.size();
    
    // Minimum 50 samples, scale up with control points
    int baseSamples = 50;
    
    if (n <= 4)
        return baseSamples;  // Cubic or less
    else if (n <= 8)
        return baseSamples + (n - 4) * 15;  // Add 15 samples per additional point
    else if (n <= 15)
        return baseSamples + 60 + (n - 8) * 20;  // Scale more for higher degrees
    else
        return baseSamples + 200 + (n - 15) * 25;  // Even more for 16-20 points
}

float BezierComponent::estimateCurvatureComplexity() const
{
    if (controlPoints.size() < 3)
        return 1.0f;
    
    // Estimate complexity based on angle changes in control polygon
    float totalAngleChange = 0.0f;
    
    for (size_t i = 1; i < controlPoints.size() - 1; i++)
    {
        auto v1 = controlPoints[i] - controlPoints[i-1];
        auto v2 = controlPoints[i+1] - controlPoints[i];
        
        float angle1 = std::atan2(v1.getY(), v1.getX());
        float angle2 = std::atan2(v2.getY(), v2.getX());
        
        float angleDiff = std::abs(angle2 - angle1);
        if (angleDiff > juce::MathConstants<float>::pi)
            angleDiff = 2 * juce::MathConstants<float>::pi - angleDiff;
            
        totalAngleChange += angleDiff;
    }
    
    return totalAngleChange;
}

// ============================================================================
// DE CASTELJAU ALGORITHM - OPTIMIZED ITERATIVE VERSION
// ============================================================================

juce::Point<float> BezierComponent::deCasteljauIterative(
    const std::vector<juce::Point<float>>& points, float t)
{
    if (points.size() == 1)
        return points[0];
    
    // Use pre-allocated buffer to avoid repeated allocations
    std::vector<juce::Point<float>> buffer1(points);
    std::vector<juce::Point<float>> buffer2;
    buffer2.reserve(points.size());
    
    std::vector<juce::Point<float>>* currentBuffer = &buffer1;
    std::vector<juce::Point<float>>* nextBuffer = &buffer2;
    
    while (currentBuffer->size() > 1)
    {
        nextBuffer->clear();
        
        const float one_minus_t = 1.0f - t;
        
        for (size_t i = 0; i < currentBuffer->size() - 1; i++)
        {
            const auto& p1 = (*currentBuffer)[i];
            const auto& p2 = (*currentBuffer)[i + 1];
            
            float x = one_minus_t * p1.getX() + t * p2.getX();
            float y = one_minus_t * p1.getY() + t * p2.getY();
            nextBuffer->emplace_back(x, y);
        }
        
        std::swap(currentBuffer, nextBuffer);
    }
    
    return (*currentBuffer)[0];
}

// Keep original recursive version for reference (but use iterative in practice)
juce::Point<float> BezierComponent::deCasteljau(
    const std::vector<juce::Point<float>>& points, float t)
{
    if (points.size() == 1)
        return points[0];
    
    std::vector<juce::Point<float>> newPoints;
    newPoints.reserve(points.size() - 1);
    
    const float one_minus_t = 1.0f - t;
    
    for (size_t i = 0; i < points.size() - 1; i++)
    {
        float x = one_minus_t * points[i].getX() + t * points[i + 1].getX();
        float y = one_minus_t * points[i].getY() + t * points[i + 1].getY();
        newPoints.emplace_back(x, y);
    }
    
    return deCasteljau(newPoints, t);
}

std::vector<std::vector<juce::Point<float>>> BezierComponent::deCasteljauWithStages(
    const std::vector<juce::Point<float>>& points, float t)
{
    std::vector<std::vector<juce::Point<float>>> stages;
    stages.reserve(points.size());
    stages.push_back(points);
    
    std::vector<juce::Point<float>> currentStage = points;
    const float one_minus_t = 1.0f - t;
    
    while (currentStage.size() > 1)
    {
        std::vector<juce::Point<float>> nextStage;
        nextStage.reserve(currentStage.size() - 1);
        
        for (size_t i = 0; i < currentStage.size() - 1; i++)
        {
            float x = one_minus_t * currentStage[i].getX() + t * currentStage[i + 1].getX();
            float y = one_minus_t * currentStage[i].getY() + t * currentStage[i + 1].getY();
            nextStage.emplace_back(x, y);
        }
        
        stages.push_back(nextStage);
        currentStage = std::move(nextStage);
    }
    
    return stages;
}

// ============================================================================
// BERNSTEIN POLYNOMIALS - WITH CACHED BINOMIAL COEFFICIENTS
// ============================================================================

void BezierComponent::updateBinomialCache(int maxN)
{
    binomialCache.clear();
    binomialCache.resize(maxN + 1);
    
    for (int n = 0; n <= maxN; n++)
    {
        binomialCache[n].resize(n + 1);
        for (int k = 0; k <= n; k++)
        {
            if (k == 0 || k == n)
                binomialCache[n][k] = 1.0f;
            else
                binomialCache[n][k] = binomialCache[n-1][k-1] + binomialCache[n-1][k];
        }
    }
}

float BezierComponent::getCachedBinomial(int n, int i)
{
    if (n < (int)binomialCache.size() && i < (int)binomialCache[n].size())
        return binomialCache[n][i];
    
    return binomialCoefficient(n, i);  // Fallback
}

float BezierComponent::binomialCoefficient(int n, int i)
{
    if (i == 0 || i == n)
        return 1.0f;
    
    float result = 1.0f;
    for (int k = 0; k < i; k++)
    {
        result *= (float)(n - k) / (float)(i - k);
    }
    return result;
}

float BezierComponent::bernsteinPolynomial(int i, int n, float t)
{
    float coeff = getCachedBinomial(n, i);
    return coeff * std::pow(1 - t, n - i) * std::pow(t, i);
}

juce::Point<float> BezierComponent::evaluateBezierBernstein(
    const std::vector<juce::Point<float>>& points, float t)
{
    int n = (int)points.size() - 1;
    float x = 0.0f;
    float y = 0.0f;
    
    // Pre-compute powers for efficiency
    std::vector<float> tPowers(n + 1);
    std::vector<float> oneMinus_t_Powers(n + 1);
    
    tPowers[0] = 1.0f;
    oneMinus_t_Powers[0] = 1.0f;
    
    for (int i = 1; i <= n; i++)
    {
        tPowers[i] = tPowers[i-1] * t;
        oneMinus_t_Powers[i] = oneMinus_t_Powers[i-1] * (1 - t);
    }
    
    for (int i = 0; i <= n; i++)
    {
        float B = getCachedBinomial(n, i) * oneMinus_t_Powers[n - i] * tPowers[i];
        x += B * points[i].getX();
        y += B * points[i].getY();
    }
    
    return juce::Point<float>(x, y);
}

// ============================================================================
// MIDPOINT SUBDIVISION
// ============================================================================

void BezierComponent::subdivisionStep(const std::vector<juce::Point<float>>& controlPts,
                                     std::vector<juce::Point<float>>& leftPts,
                                     std::vector<juce::Point<float>>& rightPts)
{
    int n = (int)controlPts.size() - 1;
    
    std::vector<std::vector<juce::Point<float>>> stages;
    stages.reserve(n + 1);
    stages.push_back(controlPts);
    
    std::vector<juce::Point<float>> currentStage = controlPts;
    
    while (currentStage.size() > 1)
    {
        std::vector<juce::Point<float>> nextStage;
        nextStage.reserve(currentStage.size() - 1);
        
        for (size_t i = 0; i < currentStage.size() - 1; i++)
        {
            float x = 0.5f * (currentStage[i].getX() + currentStage[i + 1].getX());
            float y = 0.5f * (currentStage[i].getY() + currentStage[i + 1].getY());
            nextStage.emplace_back(x, y);
        }
        
        stages.push_back(nextStage);
        currentStage = std::move(nextStage);
    }
    
    // Build left and right control points
    leftPts.clear();
    leftPts.reserve(stages.size());
    for (const auto& stage : stages)
    {
        leftPts.push_back(stage[0]);
    }
    
    rightPts.clear();
    rightPts.reserve(stages.size());
    for (int i = (int)stages.size() - 1; i >= 0; i--)
    {
        rightPts.push_back(stages[i].back());
    }
}

std::vector<juce::Point<float>> BezierComponent::generateCurveBySubdivision(
    const std::vector<juce::Point<float>>& controlPts, int k)
{
    std::vector<std::vector<juce::Point<float>>> allSegments;
    allSegments.push_back(controlPts);
    
    for (int level = 0; level < k; level++)
    {
        std::vector<std::vector<juce::Point<float>>> newSegments;
        newSegments.reserve(allSegments.size() * 2);
        
        for (const auto& segment : allSegments)
        {
            std::vector<juce::Point<float>> leftPts, rightPts;
            subdivisionStep(segment, leftPts, rightPts);
            newSegments.push_back(std::move(leftPts));
            newSegments.push_back(std::move(rightPts));
        }
        
        allSegments = std::move(newSegments);
    }
    
    // Collect curve points
    std::vector<juce::Point<float>> curvePoints;
    curvePoints.reserve(allSegments.size() + 1);
    
    for (const auto& segment : allSegments)
    {
        if (!segment.empty())
        {
            curvePoints.push_back(segment[0]);
        }
    }
    
    if (!allSegments.empty() && !allSegments.back().empty())
    {
        curvePoints.push_back(allSegments.back().back());
    }
    
    return curvePoints;
}

// ============================================================================
// CACHED PATH UPDATES
// ============================================================================

void BezierComponent::updateCachedCurvePath()
{
    cachedCurvePath.clear();
    
    if (controlPoints.size() < 2)
        return;
    
    if (currentMethod == MIDPOINT_SUBDIVISION)
    {
        auto curvePoints = generateCurveBySubdivision(controlPoints, subdivisionLevel);
        
        if (!curvePoints.empty())
        {
            cachedCurvePath.startNewSubPath(curvePoints[0]);
            for (size_t i = 1; i < curvePoints.size(); i++)
            {
                cachedCurvePath.lineTo(curvePoints[i]);
            }
        }
    }
    else
    {
        // Use adaptive sampling for smoother curves with many control points
        int numSteps = getOptimalSampleCount();
        
        for (int i = 0; i <= numSteps; i++)
        {
            float t = (float)i / (float)numSteps;
            juce::Point<float> pt;
            
            if (currentMethod == NESTED_LINEAR_INTERPOLATION)
            {
                // Use iterative version for better performance
                pt = deCasteljauIterative(controlPoints, t);
            }
            else // BERNSTEIN_POLYNOMIALS
            {
                pt = evaluateBezierBernstein(controlPoints, t);
            }
            
            if (i == 0)
                cachedCurvePath.startNewSubPath(pt);
            else
                cachedCurvePath.lineTo(pt);
        }
    }
}

void BezierComponent::updateCachedPolylinePath()
{
    cachedPolylinePath.clear();
    
    if (controlPoints.size() < 2)
        return;
    
    cachedPolylinePath.startNewSubPath(controlPoints[0]);
    for (size_t i = 1; i < controlPoints.size(); i++)
    {
        cachedPolylinePath.lineTo(controlPoints[i]);
    }
}

void BezierComponent::updateCachedShells()
{
    cachedShellStages = deCasteljauWithStages(controlPoints, tValue);
}

// ============================================================================
// DRAWING METHODS
// ============================================================================

void BezierComponent::drawBezierCurve(juce::Graphics& g)
{
    g.setColour(juce::Colours::blue);
    g.strokePath(cachedCurvePath, juce::PathStrokeType(2.5f));
}

void BezierComponent::drawControlPolyline(juce::Graphics& g)
{
    g.setColour(juce::Colours::grey);
    g.strokePath(cachedPolylinePath, juce::PathStrokeType(1.0f));
}

void BezierComponent::drawControlPoints(juce::Graphics& g)
{
    for (size_t i = 0; i < controlPoints.size(); i++)
    {
        if ((int)i == selectedPointIndex)
        {
            drawPoint(g, controlPoints[i], juce::Colours::red, 8.0f);
        }
        else
        {
            drawPoint(g, controlPoints[i], juce::Colours::darkblue, 6.0f);
        }
    }
}

void BezierComponent::drawShells(juce::Graphics& g)
{
    if (controlPoints.size() < 2)
        return;
    
    // Use cached shell stages
    const auto& stages = cachedShellStages;
    
    juce::Array<juce::Colour> stageColors;
    stageColors.add(juce::Colours::lightgrey);
    stageColors.add(juce::Colours::lightgreen);
    stageColors.add(juce::Colours::lightcoral);
    stageColors.add(juce::Colours::lightyellow);
    stageColors.add(juce::Colours::lightcyan);
    
    for (size_t stageIdx = 1; stageIdx < stages.size(); stageIdx++)
    {
        const auto& stage = stages[stageIdx];
        juce::Colour color = stageColors[stageIdx % stageColors.size()];
        
        for (size_t i = 0; i < stage.size(); i++)
        {
            if (i + 1 < stage.size())
            {
                drawLine(g, stage[i], stage[i + 1], color, 1.5f);
            }
            
            drawPoint(g, stage[i], color.darker(0.3f), 4.0f);
        }
    }
    
    // Draw final point
    if (!stages.empty() && !stages.back().empty())
    {
        drawPoint(g, stages.back()[0], juce::Colours::red, 7.0f);
    }
}

void BezierComponent::drawPoint(juce::Graphics& g, juce::Point<float> p, 
                               juce::Colour color, float radius)
{
    g.setColour(color);
    g.fillEllipse(p.getX() - radius/2, p.getY() - radius/2, radius, radius);
    g.setColour(color.darker());
    g.drawEllipse(p.getX() - radius/2, p.getY() - radius/2, radius, radius, 1.0f);
}

void BezierComponent::drawLine(juce::Graphics& g, juce::Point<float> p1, 
                              juce::Point<float> p2, juce::Colour color, float thickness)
{
    g.setColour(color);
    g.drawLine(p1.getX(), p1.getY(), p2.getX(), p2.getY(), thickness);
}

// ============================================================================
// MOUSE INTERACTION
// ============================================================================

void BezierComponent::mouseDown(const juce::MouseEvent& event)
{
    juce::Point<float> mousePos = event.position;
    selectedPointIndex = findClosestPoint(mousePos);
    repaint();
}

void BezierComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (selectedPointIndex >= 0 && selectedPointIndex < (int)controlPoints.size())
    {
        controlPoints[selectedPointIndex] = event.position;
        markCurveDirty();
        repaint();
    }
}

void BezierComponent::mouseUp(const juce::MouseEvent& event)
{
    selectedPointIndex = -1;
    repaint();
}

void BezierComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    if ((int)controlPoints.size() < MAX_CONTROL_POINTS)
    {
        addControlPoint(event.position);
        if (onControlPointsChanged)
            onControlPointsChanged();
    }
}

int BezierComponent::findClosestPoint(juce::Point<float> mousePos)
{
    int closestIdx = -1;
    float minDist = selectionRadius;
    
    for (size_t i = 0; i < controlPoints.size(); i++)
    {
        float dist = mousePos.getDistanceFrom(controlPoints[i]);
        if (dist < minDist)
        {
            minDist = dist;
            closestIdx = (int)i;
        }
    }
    
    return closestIdx;
}

// ============================================================================
// PUBLIC SETTERS
// ============================================================================

void BezierComponent::setComputationMethod(ComputationMethod method)
{
    if (currentMethod != method)
    {
        currentMethod = method;
        markCurveDirty();
        repaint();
    }
}

void BezierComponent::setSubdivisionLevel(int k)
{
    int newLevel = juce::jmax(1, juce::jmin(k, 10));
    if (subdivisionLevel != newLevel)
    {
        subdivisionLevel = newLevel;
        markCurveDirty();
        repaint();
    }
}

void BezierComponent::setTValue(float t)
{
    float newT = juce::jlimit(0.0f, 1.0f, t);
    if (tValue != newT)
    {
        tValue = newT;
        markShellsDirty();
        repaint();
    }
}

void BezierComponent::toggleShowShells(bool show)
{
    if (showShells != show)
    {
        showShells = show;
        repaint();
    }
}

void BezierComponent::toggleShowControlPolyline(bool show)
{
    if (showControlPolyline != show)
    {
        showControlPolyline = show;
        repaint();
    }
}

void BezierComponent::addControlPoint(juce::Point<float> point)
{
    if ((int)controlPoints.size() < MAX_CONTROL_POINTS)
    {
        controlPoints.push_back(point);
        
        // Update binomial cache if needed
        if (controlPoints.size() > binomialCache.size())
        {
            updateBinomialCache((int)controlPoints.size());
        }
        
        markCurveDirty();
        repaint();
    }
}

void BezierComponent::clearControlPoints()
{
    controlPoints.clear();
    initializeDefaultControlPoints();
    if (onControlPointsChanged)
        onControlPointsChanged();
    markCurveDirty();
    repaint();
}

void BezierComponent::removeLastControlPoint()
{
    if ((int)controlPoints.size() > MIN_CONTROL_POINTS)
    {
        controlPoints.pop_back();
        if (onControlPointsChanged)
            onControlPointsChanged();
        markCurveDirty();
        repaint();
    }
}
