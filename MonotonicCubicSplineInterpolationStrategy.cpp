#include "MonotonicCubicSplineInterpolationStrategy.h"

double MonotonicCubicInterpolationStrategy::sign(double x) const {
    return (x > 0) ? 1.0 : ((x < 0) ? -1.0 : 0.0);
}


double MonotonicCubicInterpolationStrategy::interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const {
    if (x.size() != y.size() || x.size() < 3) {
        throw std::invalid_argument("Need at least 3 points for monotonic cubic interpolation");
    }

    size_t n = x.size();
    std::vector<double> slopes(n - 1);
    std::vector<double> derivatives(n);

    // Calculate secant slopes
    for (size_t i = 0; i < n - 1; ++i) {
        slopes[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
    }

    // Calculate derivatives preserving monotonicity
    derivatives[0] = slopes[0];
    derivatives[n - 1] = slopes[n - 2];

    for (size_t i = 1; i < n - 1; ++i) {
        if (sign(slopes[i - 1]) != sign(slopes[i]) ||
            std::abs(slopes[i - 1]) < 1e-10 || std::abs(slopes[i]) < 1e-10) {
            derivatives[i] = 0.0;
        }
        else {
            derivatives[i] = 2.0 / (1.0 / slopes[i - 1] + 1.0 / slopes[i]);
        }
    }

    // Find segment and interpolate
    auto it = std::lower_bound(x.begin(), x.end(), targetX);

    if (it == x.begin()) return y[0];
    if (it == x.end()) return y.back();

    size_t i = std::distance(x.begin(), it) - 1;
    double h = x[i + 1] - x[i];
    double t = (targetX - x[i]) / h;

    // Hermite interpolation
    double h00 = 2 * t * t * t - 3 * t * t + 1;
    double h10 = t * t * t - 2 * t * t + t;
    double h01 = -2 * t * t * t + 3 * t * t;
    double h11 = t * t * t - t * t;

    return h00 * y[i] + h10 * h * derivatives[i] + h01 * y[i + 1] + h11 * h * derivatives[i + 1];
}

std::string MonotonicCubicInterpolationStrategy::getName() const { return "MonotonicCubic"; }
