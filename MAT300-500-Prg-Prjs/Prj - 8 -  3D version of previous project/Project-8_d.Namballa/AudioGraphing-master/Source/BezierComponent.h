#pragma once

#include <JuceHeader.h>
#include <vector>
#include <functional>

class BezierComponent : public juce::Component
{
public:
    struct Point3D
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    enum ComputationMethod
    {
        NESTED_LINEAR_INTERPOLATION,
        BERNSTEIN_POLYNOMIALS,
        MIDPOINT_SUBDIVISION
    };

    BezierComponent();
    ~BezierComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void setComputationMethod(ComputationMethod method);
    void setSubdivisionLevel(int k);
    void setTValue(float t);
    void toggleShowShells(bool show);
    void toggleShowControlPolyline(bool show);

    void setCameraYaw(float newYaw);
    void setCameraPitch(float newPitch);
    void setCameraZoom(float newZoom);
    float getCameraYaw() const { return yaw; }
    float getCameraPitch() const { return pitch; }
    float getCameraZoom() const { return zoom; }

    void setSelectedPointZ(float zValue);
    float getSelectedPointZ() const;
    int getSelectedPointIndex() const { return selectedPointIndex; }

    void addControlPoint(Point3D point);
    void addControlPoint(juce::Point<float> screenPoint);
    void removeLastControlPoint();
    void clearControlPoints();
    int getNumControlPoints() const { return static_cast<int>(controlPoints.size()); }

    std::function<void()> onControlPointsChanged;
    std::function<void(int)> onSelectedPointChanged;

private:
    static constexpr int MIN_CONTROL_POINTS = 2;
    static constexpr int MAX_CONTROL_POINTS = 20;

    std::vector<Point3D> controlPoints;
    int selectedPointIndex = -1;
    float selectionRadius = 12.0f;

    ComputationMethod currentMethod = NESTED_LINEAR_INTERPOLATION;
    int subdivisionLevel = 3;
    float tValue = 0.5f;
    bool showShells = true;
    bool showControlPolyline = true;

    float yaw = 0.65f;
    float pitch = -0.35f;
    float zoom = 1.0f;
    float sceneScale = 150.0f;
    float cameraDistance = 7.0f;

    juce::Point<float> lastDragPosition;

    std::vector<std::vector<Point3D>> cachedShellStages;
    std::vector<Point3D> cachedCurvePoints;
    std::vector<Point3D> cachedSubdivisionPoints;
    bool curveDirty = true;
    bool shellsDirty = true;

    void initializeDefaultControlPoints();
    void markCurveDirty();
    void markShellsDirty();

    Point3D lerp(const Point3D& a, const Point3D& b, float t) const;
    juce::Point<float> projectPoint(const Point3D& p) const;
    Point3D screenToWorld(juce::Point<float> screenPoint, float zPlane) const;

    Point3D deCasteljauIterative(const std::vector<Point3D>& points, float t) const;
    std::vector<std::vector<Point3D>> deCasteljauWithStages(const std::vector<Point3D>& points, float t) const;

    float binomialCoefficient(int n, int k) const;
    Point3D evaluateBezierBernstein(const std::vector<Point3D>& points, float t) const;

    void subdivisionStep(const std::vector<Point3D>& controlPts,
                         std::vector<Point3D>& leftPts,
                         std::vector<Point3D>& rightPts) const;
    std::vector<Point3D> generateCurveBySubdivision(const std::vector<Point3D>& controlPts, int k) const;

    int getOptimalSampleCount() const;
    void updateCachedCurve();
    void updateCachedShells();

    void drawAxes(juce::Graphics& g);
    void drawBezierCurve(juce::Graphics& g);
    void drawControlPolyline(juce::Graphics& g);
    void drawControlPoints(juce::Graphics& g);
    void drawShells(juce::Graphics& g);
    void drawPointLabel(juce::Graphics& g, const juce::Point<float>& p, const juce::String& text, juce::Colour colour);
    void drawPoint(juce::Graphics& g, const Point3D& p, juce::Colour color, float radius);
    void drawLine(juce::Graphics& g, const Point3D& p1, const Point3D& p2, juce::Colour color, float thickness);

    int findClosestPoint(juce::Point<float> mousePos) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BezierComponent)
};
