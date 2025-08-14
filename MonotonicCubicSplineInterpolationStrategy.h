#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "IInterpolationStrategy.h"

// Monotonic cubic spline interpolation (preserves monotonicity)
class MonotonicCubicInterpolationStrategy : public IInterpolationStrategy {

private:

    double sign(double x) const;

public:

    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    std::string getName() const override;
};
