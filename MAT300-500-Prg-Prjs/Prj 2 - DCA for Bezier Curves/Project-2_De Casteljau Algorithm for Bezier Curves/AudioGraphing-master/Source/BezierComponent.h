/*
  ==============================================================================

    BezierComponent.h
    Created for MATH 500 - Project 2
    De Casteljau Algorithm for Bezier Curves
    
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class BezierComponent : public juce::Component
{
public:
    BezierComponent();
    ~BezierComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Mouse interaction
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    
    // Computation methods
    enum ComputationMethod {
        NESTED_LINEAR_INTERPOLATION,
        BERNSTEIN_POLYNOMIALS,
        MIDPOINT_SUBDIVISION
    };
    
    void setComputationMethod(ComputationMethod method);
    void setSubdivisionLevel(int k);
    void setTValue(float t);
    void toggleShowShells(bool show);
    void toggleShowControlPolyline(bool show);
    void addControlPoint(juce::Point<float> point);
    void removeLastControlPoint();
    void clearControlPoints();
    int getNumControlPoints() const { return (int)controlPoints.size(); }
    
    // Callback for when control points change
    std::function<void()> onControlPointsChanged;
    
private:
    // Control points
    static constexpr int MIN_CONTROL_POINTS = 2;
    static constexpr int MAX_CONTROL_POINTS = 20;
    std::vector<juce::Point<float>> controlPoints;
    int selectedPointIndex;
    float selectionRadius;
    
    // Computation settings
    ComputationMethod currentMethod;
    int subdivisionLevel;
    float tValue;
    bool showShells;
    bool showControlPolyline;
    
    // OPTIMIZATION: Caching system
    bool curveDirty;
    bool shellsDirty;
    juce::Path cachedCurvePath;
    juce::Path cachedPolylinePath;
    std::vector<std::vector<juce::Point<float>>> cachedShellStages;
    
    // OPTIMIZATION: Pre-allocated buffers
    std::vector<juce::Point<float>> tempPointBuffer;
    
    // OPTIMIZATION: Adaptive sampling
    int getOptimalSampleCount() const;
    float estimateCurvatureComplexity() const;
    
    // Mark cache as dirty
    void markCurveDirty();
    void markShellsDirty();
    
    // De Casteljau Algorithm - Nested Linear Interpolation
    juce::Point<float> deCasteljau(const std::vector<juce::Point<float>>& points, float t);
    std::vector<std::vector<juce::Point<float>>> deCasteljauWithStages(
        const std::vector<juce::Point<float>>& points, float t);
    
    // OPTIMIZATION: Iterative version to avoid recursion overhead
    juce::Point<float> deCasteljauIterative(const std::vector<juce::Point<float>>& points, float t);
    
    // Bernstein Polynomials
    float bernsteinPolynomial(int i, int n, float t);
    float binomialCoefficient(int n, int i);
    juce::Point<float> evaluateBezierBernstein(const std::vector<juce::Point<float>>& points, float t);
    
    // OPTIMIZATION: Cached binomial coefficients
    std::vector<std::vector<float>> binomialCache;
    void updateBinomialCache(int maxN);
    float getCachedBinomial(int n, int i);
    
    // Midpoint Subdivision
    void subdivisionStep(const std::vector<juce::Point<float>>& controlPts,
                        std::vector<juce::Point<float>>& leftPts,
                        std::vector<juce::Point<float>>& rightPts);
    std::vector<juce::Point<float>> generateCurveBySubdivision(
        const std::vector<juce::Point<float>>& controlPts, int k);
    
    // Drawing helpers (with caching)
    void updateCachedCurvePath();
    void updateCachedPolylinePath();
    void updateCachedShells();
    
    void drawControlPoints(juce::Graphics& g);
    void drawControlPolyline(juce::Graphics& g);
    void drawBezierCurve(juce::Graphics& g);
    void drawShells(juce::Graphics& g);
    void drawPoint(juce::Graphics& g, juce::Point<float> p, juce::Colour color, float radius);
    void drawLine(juce::Graphics& g, juce::Point<float> p1, juce::Point<float> p2, 
                  juce::Colour color, float thickness);
    
    // Utility
    int findClosestPoint(juce::Point<float> mousePos);
    void initializeDefaultControlPoints();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BezierComponent)
};
