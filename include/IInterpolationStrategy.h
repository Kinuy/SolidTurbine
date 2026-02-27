#pragma once

/**
 * @brief Interface for interpolation algorithms using strategy pattern
 *
 * IInterpolationStrategy defines the contract for mathematical interpolation
 * methods, enabling pluggable algorithms for estimating values between
 * known data points. Supports various interpolation techniques through
 * polymorphic implementations.
 *
 * ## Key Features
 * - **Strategy Pattern**: Swappable interpolation algorithms
 * - **Mathematical Accuracy**: Different methods for different data characteristics
 * - **Performance Flexibility**: Choose algorithms based on speed vs accuracy needs
 * - **Identifier Support**: Named strategies for configuration and debugging
 *
 * ## Common Implementations
 * - **LinearInterpolationStrategy**: Fast, simple linear interpolation
 * - **CubicSplineInterpolationStrategy**: Smooth curves with continuous derivatives
 * - **AkimaSplineInterpolationStrategy**: Reduced oscillation splines
 * - **MonotonicCubicInterpolationStrategy**: Preserves monotonicity
 *
 * @see BladeGeometryInterpolator for typical usage context
 * @see CubicSplineInterpolationStrategy for a concrete implementation
 *
 * @example
 * ```cpp
 * std::unique_ptr<IInterpolationStrategy> strategy =
 *     std::make_unique<CubicSplineInterpolationStrategy>();
 *
 * double result = strategy->interpolate(xData, yData, 2.5);
 * std::string method = strategy->getName();
 * ```
 */
class IInterpolationStrategy {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IInterpolationStrategy() = default;

    /**
     * @brief Interpolates y-value at target x-coordinate using strategy algorithm
     * @param x Vector of x-coordinates (should be sorted in ascending order)
     * @param y Vector of corresponding y-values (same size as x)
     * @param targetX X-coordinate where interpolated value is desired
     * @return Interpolated y-value at targetX
     * @throws std::invalid_argument if x and y vectors have different sizes or insufficient data
     */
    virtual double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const = 0;

    /**
     * @brief Gets the name identifier for this interpolation strategy
     * @return String name identifying the interpolation method
     */
    virtual std::string getName() const = 0;

};
