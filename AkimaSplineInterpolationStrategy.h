#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "IInterpolationStrategy.h"

// Akima spline interpolation strategy (more robust than cubic for noisy data)
class AkimaSplineInterpolationStrategy : public IInterpolationStrategy {

private:

    double calculateSlope(double x1, double y1, double x2, double y2) const;

    double akimaWeight(double s1, double s2, double s3, double s4) const;

public:

    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;
    std::string getName() const override;

};

