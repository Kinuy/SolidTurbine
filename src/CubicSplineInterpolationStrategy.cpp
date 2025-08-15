#include "CubicSplineInterpolationStrategy.h"


std::vector<CubicSplineInterpolationStrategy::SplineSegment> CubicSplineInterpolationStrategy::calculateSplineCoefficients(const std::vector<double>& x, const std::vector<double>& y) const {
    size_t n = x.size();
    if (n < 3) throw std::invalid_argument("Need at least 3 points for cubic spline");

    std::vector<SplineSegment> segments(n - 1);
    std::vector<double> h(n - 1), alpha(n - 1), l(n), mu(n), z(n);

    // Calculate step sizes
    for (size_t i = 0; i < n - 1; ++i) {
        h[i] = x[i + 1] - x[i];
    }

    // Calculate alpha values
    for (size_t i = 1; i < n - 1; ++i) {
        alpha[i] = (3.0 / h[i]) * (y[i + 1] - y[i]) - (3.0 / h[i - 1]) * (y[i] - y[i - 1]);
    }

    // Solve tridiagonal system (natural spline: second derivatives at endpoints = 0)
    l[0] = 1.0; mu[0] = 0.0; z[0] = 0.0;

    for (size_t i = 1; i < n - 1; ++i) {
        l[i] = 2.0 * (x[i + 1] - x[i - 1]) - h[i - 1] * mu[i - 1];
        mu[i] = h[i] / l[i];
        z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
    }

    l[n - 1] = 1.0; z[n - 1] = 0.0;

    std::vector<double> c(n);
    c[n - 1] = 0.0;

    for (int j = n - 2; j >= 0; --j) {
        c[j] = z[j] - mu[j] * c[j + 1];
    }

    // Calculate spline coefficients
    for (size_t j = 0; j < n - 1; ++j) {
        segments[j].a = y[j];
        segments[j].b = (y[j + 1] - y[j]) / h[j] - h[j] * (2.0 * c[j] + c[j + 1]) / 3.0;
        segments[j].c = c[j];
        segments[j].d = (c[j + 1] - c[j]) / (3.0 * h[j]);
        segments[j].x1 = x[j];
        segments[j].x2 = x[j + 1];
    }

    return segments;
}

double CubicSplineInterpolationStrategy::interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const  {
    if (x.size() != y.size() || x.size() < 3) {
        throw std::invalid_argument("Need at least 3 points for cubic spline interpolation");
    }

    auto segments = calculateSplineCoefficients(x, y);

    // Find appropriate segment
    for (const auto& segment : segments) {
        if (targetX >= segment.x1 && targetX <= segment.x2) {
            double dx = targetX - segment.x1;
            return segment.a + segment.b * dx + segment.c * dx * dx + segment.d * dx * dx * dx;
        }
    }

    // Outside range - use linear extrapolation
    if (targetX < x[0]) {
        const auto& segment = segments[0];
        double dx = targetX - segment.x1;
        return segment.a + segment.b * dx;
    }
    else {
        const auto& segment = segments.back();
        double dx = targetX - segment.x1;
        return segment.a + segment.b * dx + segment.c * dx * dx + segment.d * dx * dx * dx;
    }
}

std::string CubicSplineInterpolationStrategy::getName() const { return "CubicSpline"; }
