#pragma once
#include <JuceHeader.h>
#include <vector>

//==============================================================================
// 1D Newton Interpolator using divided differences
//==============================================================================
class NewtonInterpolator1D
{
public:
    void clear()
    {
        nodes.clear();
        coeffs.clear();
        values.clear();
    }

    void setData(const std::vector<double>& tNodes, const std::vector<double>& fValues)
    {
        nodes = tNodes;
        values = fValues;
        rebuildCoefficients();
    }

    int size() const { return (int)nodes.size(); }

    const std::vector<double>& getNodes()  const { return nodes; }
    const std::vector<double>& getCoeffs() const { return coeffs; }

    // p(t) = a0 + (t-t0)[a1 + (t-t1)[a2 + ...]]
    double evaluate(double t) const
    {
        const int n = (int)coeffs.size();
        if (n == 0) return 0.0;

        double result = coeffs[n - 1];
        for (int k = n - 2; k >= 0; --k)
            result = coeffs[k] + (t - nodes[(size_t)k]) * result;

        return result;
    }

private:
    void rebuildCoefficients()
    {
        const int n = (int)nodes.size();
        coeffs.clear();
        if (n == 0) return;

        std::vector<double> temp = values;

        coeffs.resize((size_t)n);
        coeffs[0] = temp[0];

        for (int order = 1; order < n; ++order)
        {
            for (int i = 0; i < n - order; ++i)
            {
                const double denom = nodes[(size_t)(i + order)] - nodes[(size_t)i];
                temp[(size_t)i] = (temp[(size_t)(i + 1)] - temp[(size_t)i]) / denom;
            }
            coeffs[(size_t)order] = temp[0];
        }
    }

    std::vector<double> nodes;
    std::vector<double> values;
    std::vector<double> coeffs;
};

//==============================================================================
// MainContentComponent
//==============================================================================
class MainContentComponent : public juce::Component
{
public:
    MainContentComponent();
    ~MainContentComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // to detect holding 'C'
    bool keyStateChanged(bool isKeyDown) override;

private:
    void rebuildInterpolators();
    int  findPointNear(juce::Point<float> p, float radius) const;
    juce::Path buildCurvePath() const;

    void drawInfoPanel(juce::Graphics& g, juce::Rectangle<int> area) const;

    // Cut-mode behavior (hold C):
    // - remove selected point from interpolation set while dragging
    // - on release append as last point and recompute
    void beginCutDrag(int index);
    void commitCutDragAppendLast();

    std::vector<juce::Point<float>> points;  // interpolation set

    // Normal drag: point stays in set and updates live
    int  selectedIndex = -1;
    bool dragging = false;

    // Key state for cut-mode
    bool cutKeyDown = false;

    // When cut-mode dragging, the selected point is temporarily removed:
    bool hasTempPoint = false;
    juce::Point<float> tempPoint;

    NewtonInterpolator1D interpX;
    NewtonInterpolator1D interpY;

    static constexpr float pointRadius = 7.0f;
    static constexpr float pickRadius = 12.0f;
    static constexpr int   infoPanelHeight = 110;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};