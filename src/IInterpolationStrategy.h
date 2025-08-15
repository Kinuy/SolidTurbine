#pragma once

// Interface for different interpolation strategies
class IInterpolationStrategy {
public:
    virtual ~IInterpolationStrategy() = default;
    virtual double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const = 0;
    virtual std::string getName() const = 0;
};
