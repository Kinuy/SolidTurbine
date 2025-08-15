#include "AkimaSplineInterpolationStrategy.h"

double AkimaSplineInterpolationStrategy::calculateSlope(double x1, double y1, double x2, double y2) const {
    return (y2 - y1) / (x2 - x1);
}

double AkimaSplineInterpolationStrategy::akimaWeight(double s1, double s2, double s3, double s4) const {
    double w1 = std::abs(s4 - s3);
    double w2 = std::abs(s2 - s1);
    double denom = w1 + w2;

    if (denom < 1e-10) {
        return 0.5 * (s2 + s3); // Average when weights are very small
    }

    return (w1 * s2 + w2 * s3) / denom;
}


double AkimaSplineInterpolationStrategy::interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const {
    if (x.size() != y.size() || x.size() < 3) {
        throw std::invalid_argument("Need at least 3 points for Akima spline interpolation");
    }

    size_t n = x.size();
    std::vector<double> slopes(n + 3);
    std::vector<double> derivatives(n);

    // Calculate slopes including extended points
    for (size_t i = 0; i < n - 1; ++i) {
        slopes[i + 2] = calculateSlope(x[i], y[i], x[i + 1], y[i + 1]);
    }

    // Extend slopes at boundaries
    slopes[1] = 2.0 * slopes[2] - slopes[3];
    slopes[0] = 2.0 * slopes[1] - slopes[2];
    slopes[n + 1] = 2.0 * slopes[n] - slopes[n - 1];
    slopes[n + 2] = 2.0 * slopes[n + 1] - slopes[n];

    // Calculate Akima derivatives
    for (size_t i = 0; i < n; ++i) {
        derivatives[i] = akimaWeight(slopes[i], slopes[i + 1], slopes[i + 2], slopes[i + 3]);
    }

    // Find segment and interpolate using Hermite polynomials
    auto it = std::lower_bound(x.begin(), x.end(), targetX);

    if (it == x.begin()) return y[0];
    if (it == x.end()) return y.back();

    size_t i = std::distance(x.begin(), it) - 1;
    double h = x[i + 1] - x[i];
    double t = (targetX - x[i]) / h;

    // Hermite basis functions
    double h00 = 2 * t * t * t - 3 * t * t + 1;
    double h10 = t * t * t - 2 * t * t + t;
    double h01 = -2 * t * t * t + 3 * t * t;
    double h11 = t * t * t - t * t;

    return h00 * y[i] + h10 * h * derivatives[i] + h01 * y[i + 1] + h11 * h * derivatives[i + 1];
}

std::string AkimaSplineInterpolationStrategy::getName() const { return "AkimaSpline"; }