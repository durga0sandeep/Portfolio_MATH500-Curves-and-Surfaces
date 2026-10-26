#pragma once

#include <JuceHeader.h>
#include <vector>

class MainContentComponent : public juce::Component,
                             private juce::Slider::Listener,
                             private juce::Button::Listener,
                             private juce::TextEditor::Listener,
                             private juce::ComboBox::Listener
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

    struct CurveInfo
    {
        double domainStart = 0.0;
        double domainEnd = 1.0;
        bool valid = false;
    };

    struct ShellData
    {
        std::vector<std::vector<juce::Point<double>>> levels;
        std::vector<int> activeControlIndices;
        juce::Point<double> point { 0.0, 0.0 };
        bool valid = false;
    };

    enum class KnotPreset
    {
        openUniform = 1,
        uniform = 2,
        custom = 3
    };

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void initialiseUi();
    void resetProjectVIIConfiguration();
    void rebuildSplineSetup();
    void buildDefaultControlPoints();
    void rebuildOpenUniformKnots();
    void rebuildUniformKnots();
    void refreshUiText();
    void updateTSliderRange(bool preserveNormalizedPosition);
    void applyEditedKnotSequence();
    bool parseKnotEditorText(std::vector<double>& parsedKnots, juce::String& errorMessage) const;
    bool validateKnotSequence(const std::vector<double>& knots, juce::String& errorMessage) const;
    void setStatusMessage(const juce::String& message, bool isError);

    void drawControls(juce::Graphics& g, juce::Rectangle<int> area);
    void drawCanvas(juce::Graphics& g, juce::Rectangle<int> area);
    void drawGrid(juce::Graphics& g, const PlotArea& plot);
    void drawCurve(juce::Graphics& g, const PlotArea& plot);
    void drawControlPolygon(juce::Graphics& g,
                            const PlotArea& plot,
                            const std::vector<juce::Point<double>>& points,
                            juce::Colour colour,
                            const juce::String& prefix,
                            const std::vector<int>* highlightIndices = nullptr);
    void drawShells(juce::Graphics& g, const PlotArea& plot, const ShellData& shells);
    void drawStatusBar(juce::Graphics& g, juce::Rectangle<int> area);

    PlotArea makePlotArea(juce::Rectangle<int> area) const;
    juce::Point<float> worldToScreen(const PlotArea& plot, juce::Point<double> p) const;
    juce::Point<double> screenToWorld(const PlotArea& plot, juce::Point<float> p) const;

    int findControlPointNear(const PlotArea& plot, juce::Point<float> mousePos) const;
    void dragActiveControlPointTo(const PlotArea& plot, juce::Point<float> mousePos);

    CurveInfo getCurveInfo() const;
    double mapSampleToCurveParameter(double s) const;
    double getCurrentT() const;
    double getCurrentTNormalized() const;
    juce::Point<double> evaluateCurrentCurve(double t) const;
    juce::Point<double> evaluateDeBoorCurve(const std::vector<juce::Point<double>>& controlPoints,
                                            const std::vector<double>& knots,
                                            int splineDegree,
                                            double t) const;
    ShellData computeShellData(double t) const;
    int findKnotSpan(const std::vector<double>& knots, int splineDegree, int lastControlIndex, double t) const;

    static double clamp01(double value);
    static juce::String knotSequenceToString(const std::vector<double>& knots);
    static juce::String knotPresetToString(KnotPreset preset);

    int degree = 3;
    int controlPointCount = 7;
    int intervalCount = 4;
    int selectedPoint = -1;
    bool draggingPoint = false;
    KnotPreset currentPreset = KnotPreset::openUniform;
    bool statusIsError = false;
    juce::String statusMessage;

    std::vector<juce::Point<double>> controlPoints;
    std::vector<double> currentKnots;

    juce::Label degreeLabel;
    juce::Slider degreeSlider;

    juce::Label controlPointLabel;
    juce::Slider controlPointSlider;

    juce::Label intervalLabel;
    juce::Slider intervalSlider;

    juce::Label tLabel;
    juce::Slider tSlider;

    juce::TextButton resetButton { "Reset Project VII" };
    juce::TextButton openUniformButton { "Use Open Uniform Knots" };
    juce::TextButton applyKnotButton { "Apply Knots" };

    juce::Label knotLabel;
    juce::TextEditor knotEditor;
    juce::ComboBox knotPresetBox;
    juce::Label modeLabel;

    static constexpr int topPanelHeight = 280;
    static constexpr int statusBarHeight = 36;
    static constexpr float pointRadius = 7.0f;
    static constexpr int curveSamples = 900;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
