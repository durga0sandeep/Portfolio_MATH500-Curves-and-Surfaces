#pragma once

#include <JuceHeader.h>
#include <vector>

class MainContentComponent : public juce::Component,
                             private juce::Slider::Listener,
                             private juce::Button::Listener
{
public:
    MainContentComponent();
    ~MainContentComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    struct PlotArea
    {
        juce::Rectangle<float> rect;
    };

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

    void initialiseUi();
    void resetToDefaultBezierConfiguration();
    void rebuildBezierKnots();
    std::vector<double> buildPartVINewDefaultKnots() const;
    void switchToPartVIKnotMode();
    void switchToBezierMode();
    void rebuildDeBoorPointsFromBezier();
    void refreshKnotEditor();

    void drawControls(juce::Graphics& g, juce::Rectangle<int> area);
    void drawCanvas(juce::Graphics& g, juce::Rectangle<int> area);
    void drawGrid(juce::Graphics& g, const PlotArea& plot);
    void drawCurve(juce::Graphics& g, const PlotArea& plot);
    void drawControlPolygon(juce::Graphics& g,
                            const PlotArea& plot,
                            const std::vector<juce::Point<double>>& points,
                            juce::Colour colour,
                            bool active,
                            const juce::String& prefix);
    void drawStatusBar(juce::Graphics& g, juce::Rectangle<int> area);

    PlotArea makePlotArea(juce::Rectangle<int> area) const;
    juce::Point<float> worldToScreen(const PlotArea& plot, juce::Point<double> p) const;
    juce::Point<double> screenToWorld(const PlotArea& plot, juce::Point<float> p) const;

    std::vector<juce::Point<double>>& getActiveControlPoints();
    const std::vector<juce::Point<double>>& getActiveControlPoints() const;
    int findControlPointNear(const PlotArea& plot, juce::Point<float> mousePos) const;
    void dragActiveControlPointTo(const PlotArea& plot, juce::Point<float> mousePos);

    juce::Point<double> evaluateCurrentCurve(double t) const;
    juce::Point<double> evaluateDeBoorCurve(const std::vector<juce::Point<double>>& controlPoints,
                                            const std::vector<double>& knots,
                                            double t) const;
    juce::Point<double> evaluateBezierByDeCasteljau(double t) const;
    juce::Point<double> evaluateBlossom(const std::vector<double>& u) const;
    static double clamp01(double value);
    static juce::String knotSequenceToString(const std::vector<double>& knots);

    int degree = 3;
    bool usingPartVIKnots = false;
    int selectedPoint = -1;
    bool draggingPoint = false;

    std::vector<juce::Point<double>> bezierPoints;
    std::vector<juce::Point<double>> deBoorPoints;
    std::vector<double> currentKnots;

    juce::Label degreeLabel;
    juce::Slider degreeSlider;

    juce::TextButton resetBezierButton { "Reset Bezier Points" };
    juce::TextButton usePartVIKnotsButton { "Use New Default Knot Sequence" };
    juce::TextButton showBezierModeButton { "Show Bezier Mode" };

    juce::Label knotLabel;
    juce::TextEditor knotEditor;
    juce::Label modeLabel;

    static constexpr int topPanelHeight = 138;
    static constexpr int statusBarHeight = 34;
    static constexpr float pointRadius = 7.0f;
    static constexpr int curveSamples = 700;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
