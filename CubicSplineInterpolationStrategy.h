#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include "IInterpolationStrategy.h"


// Cubic spline interpolation strategy
class CubicSplineInterpolationStrategy : public IInterpolationStrategy {

private:

    struct SplineSegment {
        double a, b, c, d; // Coefficients for: a + b*x + c*x² + d*x³
        double x1, x2;     // Valid range for this segment
    };

    std::vector<SplineSegment> calculateSplineCoefficients(const std::vector<double>& x, const std::vector<double>& y) const;

public:

    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    std::string getName() const override;

};

