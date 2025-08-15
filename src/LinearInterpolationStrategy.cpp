#include "LinearInterpolationStrategy.h"


double LinearInterpolationStrategy::interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const {
    if (x.size() != y.size() || x.size() < 2) {
        throw std::invalid_argument("Invalid data for linear interpolation");
    }

    // Find surrounding points
    auto it = std::lower_bound(x.begin(), x.end(), targetX);

    if (it == x.begin()) return y[0];
    if (it == x.end()) return y.back();

    size_t upperIdx = std::distance(x.begin(), it);
    size_t lowerIdx = upperIdx - 1;

    double x1 = x[lowerIdx], x2 = x[upperIdx];
    double y1 = y[lowerIdx], y2 = y[upperIdx];

    double factor = (targetX - x1) / (x2 - x1);
    return y1 + factor * (y2 - y1);
}

std::string LinearInterpolationStrategy::getName() const { return "Linear"; }
