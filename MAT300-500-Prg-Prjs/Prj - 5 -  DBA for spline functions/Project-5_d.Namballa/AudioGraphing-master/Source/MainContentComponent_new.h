#pragma once

#include <JuceHeader.h>
#include <vector>

class MainContentComponent : public juce::Component,
                             private juce::Slider::Listener,
                             private juce::ComboBox::Listener,
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
    enum class EvaluationMethod
    {
        deBoor = 1,
        dividedDifferences = 2,
        shiftedPowerFunctions = 3
    };

    struct PlotArea
    {
        juce::Rectangle<float> rect;
        double xMin = 0.0;
        double xMax = 1.0;
        double yMin = -3.0;
        double yMax = 3.0;
    };

    // UI callbacks
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;

    // Setup/state
    void initialiseUi();
    void resetCoefficients();
    void syncCoefficientCount(bool preserveExisting);
    int  getCoefficientCount() const;
    double getDefaultIntervalCountForDegree(int degree) const;

    // Drawing
    void drawControls(juce::Graphics& g, juce::Rectangle<int> area);
    void drawPlot(juce::Graphics& g, juce::Rectangle<int> area);
    void drawAxes(juce::Graphics& g, const PlotArea& plot);
    void drawCurve(juce::Graphics& g, const PlotArea& plot);
    void drawCoefficientPolyline(juce::Graphics& g, const PlotArea& plot);
    void drawCoefficientDots(juce::Graphics& g, const PlotArea& plot);
    void drawStatusText(juce::Graphics& g, juce::Rectangle<int> area);

    // Coordinate conversion
    PlotArea makePlotArea(juce::Rectangle<int> area) const;
    juce::Point<float> worldToScreen(const PlotArea& plot, double x, double y) const;
    juce::Point<double> screenToWorld(const PlotArea& plot, juce::Point<float> p) const;

    // Interaction
    int findCoefficientNear(const PlotArea& plot, juce::Point<float> p) const;
    double getCoefficientCenterX(int index) const;
    void clampSelectedCoefficientY(const PlotArea& plot, juce::Point<float> mousePos);

    // Evaluation
    double evaluateSpline(double t) const;
    double evaluateByDeBoor(double t) const;
    double evaluateByDividedDifferences(double t) const;
    double evaluateByShiftedPowers(double t) const;
    double evaluateBasisShiftedPower(int i, int d, double t) const;
    double evaluateBasisDividedDifference(int i, int d, double t) const;
    static double shiftedPower(double x, int d);
    static double binomial(int n, int k);
    static double factorial(int n);
    static double recursiveDividedDifference(const std::vector<double>& xs,
                                             int left,
                                             int right,
                                             const std::function<double(double)>& f);

    // State
    int degree = 3;
    int intervalsN = 13; // default N = d + 10
    std::vector<double> coefficients;
    EvaluationMethod currentMethod = EvaluationMethod::deBoor;

    int selectedCoeff = -1;
    bool draggingCoeff = false;

    // UI widgets
    juce::Slider degreeSlider;
    juce::Label degreeLabel;

    juce::Slider intervalSlider;
    juce::Label intervalLabel;

    juce::ComboBox methodCombo;
    juce::Label methodLabel;

    juce::TextButton resetButton { "Reset Coefficients" };

    // Layout constants
    static constexpr int topPanelHeight = 110;
    static constexpr float pointRadius = 7.0f;
    static constexpr int curveSamples = 900;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
