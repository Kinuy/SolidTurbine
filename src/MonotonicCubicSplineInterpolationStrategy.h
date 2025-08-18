#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "IInterpolationStrategy.h"

/**
 * @brief Monotonic cubic interpolation strategy preserving data trends
 *
 * MonotonicCubicInterpolationStrategy implements cubic interpolation while
 * preserving the monotonic behavior of input data. Prevents oscillations
 * and overshooting that can occur with standard cubic splines, making it
 * ideal for physical data where trend preservation is critical.
 *
 * ## Key Features
 * - **Monotonicity Preservation**: Maintains increasing/decreasing trends
 * - **Smooth Curves**: Cubic interpolation for smooth transitions
 * - **Overshoot Prevention**: Avoids artificial extrema between data points
 * - **Physical Realism**: Suitable for performance curves and physical measurements
 *
 * ## Algorithm Approach
 * Uses Hermite interpolation with specially computed derivatives that
 * respect the monotonic properties of adjacent data segments. Sets
 * derivatives to zero at points where monotonicity would be violated.
 *
 * ## Best Use Cases
 * - Aerodynamic performance curves (lift, drag coefficients)
 * - Efficiency curves and power output data
 * - Physical measurements requiring trend preservation
 * - Data where artificial oscillations would be problematic
 *
 * @see IInterpolationStrategy for the base interface
 * @see CubicSplineInterpolationStrategy for standard cubic splines
 *
 * @example
 * ```cpp
 * auto strategy = std::make_unique<MonotonicCubicInterpolationStrategy>();
 * double result = strategy->interpolate(xData, performanceData, 2.5);
 * // Result preserves monotonic trends in performance data
 * ```
 */
class MonotonicCubicInterpolationStrategy : public IInterpolationStrategy {

private:

    /**
     * @brief Utility function returning the sign of a number
     * @param x Input value
     * @return 1.0 for positive, -1.0 for negative, 0.0 for zero
     */
    double sign(double x) const;

public:

    /**
     * @brief Performs monotonic cubic interpolation preserving data trends
     *
     * Uses Hermite interpolation with derivative calculation that preserves
     * monotonicity of the input data. Prevents oscillations and overshooting
     * that can occur with standard cubic splines.
     *
     * ## Algorithm Steps
     * 1. Calculate secant slopes between adjacent points
     * 2. Compute derivatives while preserving monotonicity
     * 3. Apply Hermite interpolation for smooth curves
     *
     * @param x Vector of x-coordinates (should be sorted)
     * @param y Vector of corresponding y-values
     * @param targetX X-coordinate for interpolation
     * @return Interpolated y-value preserving monotonic behavior
     * @throws std::invalid_argument if insufficient data points or size mismatch
     *
     * @note Particularly useful for performance curves and physical data
     *       where monotonic trends must be preserved
     */
    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    /**
     * @brief Gets the strategy name identifier
     * @return The string "MonotonicCubic"
     */
    std::string getName() const override;
};
