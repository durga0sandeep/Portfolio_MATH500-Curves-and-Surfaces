#pragma once
#include <JuceHeader.h>
#include <vector>

//==============================================================================
// NaturalCubicSpline1D
//
// Fits a natural cubic spline using the truncated power basis:
//
//   f(t) = a0 + a1*t + a2*t^2 + a3*t^3
//          + b1*(t-1)^3_+  + b2*(t-2)^3_+  + ... + b_{k-1}*(t-(k-1))^3_+
//
// For n interpolation points at integer nodes t = 0, 1, ..., k  (k = n-1),
// there are n + 2 unknowns: [a0, a1, a2, a3, b1, ..., b_{k-1}].
//
// Two extra equations from the natural spline boundary conditions:
//   f''(0) = 0   and   f''(k) = 0
//
// The (n+2) x (n+2) linear system is solved by Gaussian elimination
// with partial pivoting.
//==============================================================================
class NaturalCubicSpline1D
{
public:
    void clear()
    {
        nodes_.clear();
        coeffs_.clear();
    }

    void setData(const std::vector<double>& tNodes,
        const std::vector<double>& fValues)
    {
        nodes_ = tNodes;
        solve(fValues);
    }

    int size() const { return (int)nodes_.size(); }

    const std::vector<double>& getCoeffs() const { return coeffs_; }

    // Evaluate spline at parameter t
    double evaluate(double t) const
    {
        if (coeffs_.empty()) return 0.0;

        const int k = (int)nodes_.size() - 1;

        double val = coeffs_[0]
            + coeffs_[1] * t
            + coeffs_[2] * t * t
            + coeffs_[3] * t * t * t;

        // b_j * (t - j)^3_+  for j = 1 ... k-1
        for (int j = 1; j <= k - 1; ++j)
        {
            const double d = t - (double)j;
            if (d > 0.0)
                val += coeffs_[3 + (size_t)j] * d * d * d;
        }
        return val;
    }

private:
    // Truncated power (t - c)^3_+  at t
    static double tp3(double t, double c)
    {
        const double d = t - c;
        return (d > 0.0) ? d * d * d : 0.0;
    }

    // Second derivative of (t - c)^3_+  at t
    static double tp3dd(double t, double c)
    {
        const double d = t - c;
        return (d > 0.0) ? 6.0 * d : 0.0;
    }

    void solve(const std::vector<double>& f)
    {
        coeffs_.clear();
        const int n = (int)nodes_.size();
        if (n == 0) return;
        if (n == 1) { coeffs_ = { f[0], 0.0, 0.0, 0.0 }; return; }

        const int k = n - 1;
        const int sz = n + 2;   // system dimension

        // Augmented matrix [A | rhs]
        std::vector<std::vector<double>> A(sz, std::vector<double>(sz + 1, 0.0));

        // Interpolation equations: one per node
        for (int i = 0; i < n; ++i)
        {
            const double ti = nodes_[(size_t)i];
            A[(size_t)i][0] = 1.0;
            A[(size_t)i][1] = ti;
            A[(size_t)i][2] = ti * ti;
            A[(size_t)i][3] = ti * ti * ti;

            for (int j = 1; j <= k - 1; ++j)
                A[(size_t)i][(size_t)(3 + j)] = tp3(ti, (double)j);

            A[(size_t)i][(size_t)sz] = f[(size_t)i];
        }

        // Natural BC: f''(0) = 0  =>  2*a2 = 0
        A[(size_t)n][2] = 2.0;
        A[(size_t)n][(size_t)sz] = 0.0;

        // Natural BC: f''(k) = 0  =>  2*a2 + 6*a3*k + sum b_j*6*(k-j) = 0
        {
            const double tk = (double)k;
            A[(size_t)(n + 1)][2] = 2.0;
            A[(size_t)(n + 1)][3] = 6.0 * tk;
            for (int j = 1; j <= k - 1; ++j)
                A[(size_t)(n + 1)][(size_t)(3 + j)] = tp3dd(tk, (double)j);
            A[(size_t)(n + 1)][(size_t)sz] = 0.0;
        }

        // Gaussian elimination with partial pivoting
        for (int col = 0; col < sz; ++col)
        {
            int pivot = col;
            double maxVal = std::abs(A[(size_t)col][(size_t)col]);
            for (int row = col + 1; row < sz; ++row)
            {
                double v = std::abs(A[(size_t)row][(size_t)col]);
                if (v > maxVal) { maxVal = v; pivot = row; }
            }
            if (pivot != col)
                std::swap(A[(size_t)col], A[(size_t)pivot]);

            const double diag = A[(size_t)col][(size_t)col];
            if (std::abs(diag) < 1e-14) continue;

            for (int row = col + 1; row < sz; ++row)
            {
                const double factor = A[(size_t)row][(size_t)col] / diag;
                for (int c2 = col; c2 <= sz; ++c2)
                    A[(size_t)row][(size_t)c2] -= factor * A[(size_t)col][(size_t)c2];
            }
        }

        // Back substitution
        coeffs_.resize((size_t)sz, 0.0);
        for (int row = sz - 1; row >= 0; --row)
        {
            double rhs = A[(size_t)row][(size_t)sz];
            for (int c2 = row + 1; c2 < sz; ++c2)
                rhs -= A[(size_t)row][(size_t)c2] * coeffs_[(size_t)c2];
            const double diag = A[(size_t)row][(size_t)row];
            coeffs_[(size_t)row] = (std::abs(diag) > 1e-14) ? rhs / diag : 0.0;
        }
    }

    std::vector<double> nodes_;
    std::vector<double> coeffs_; // [a0, a1, a2, a3, b1, ..., b_{k-1}]
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

private:
    void rebuildSplines();
    int  findPointNear(juce::Point<float> p, float radius) const;
    juce::Path buildCurvePath() const;
    void drawInfoPanel(juce::Graphics& g, juce::Rectangle<int> area) const;

    std::vector<juce::Point<float>> points;

    int  selectedIndex = -1;
    bool dragging = false;

    NaturalCubicSpline1D splineX;
    NaturalCubicSpline1D splineY;

    static constexpr float pointRadius = 7.0f;
    static constexpr float pickRadius = 12.0f;
    static constexpr int   infoPanelHeight = 120;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
