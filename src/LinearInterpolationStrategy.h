#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "IInterpolationStrategy.h"


// Linear interpolation strategy
class LinearInterpolationStrategy : public IInterpolationStrategy {

public:

    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    std::string getName() const override;
};

