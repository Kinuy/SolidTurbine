#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "IInterpolationStrategy.h"


/**
 * @brief Simple linear interpolation strategy for fast approximations
 *
 * LinearInterpolationStrategy implements the simplest form of interpolation
 * by connecting adjacent data points with straight lines. Provides fast
 * computation with minimal memory requirements, suitable for applications
 * where speed is prioritized over smoothness.
 *
 * ## Key Features
 * - **Fast Computation**: O(log n) lookup with simple linear calculation
 * - **Memory Efficient**: No additional data structures or preprocessing
 * - **Predictable Behavior**: No oscillations or overshooting
 * - **Boundary Handling**: Returns endpoint values for out-of-range queries
 *
 * ## Characteristics
 * - **Continuity**: C⁰ continuous (function values match at data points)
 * - **Smoothness**: Not smooth - creates angular transitions at data points
 * - **Accuracy**: Good for slowly varying data, less accurate for curved data
 * - **Stability**: Always stable, no numerical issues
 *
 * ## Best Use Cases
 * - Lookup tables with dense, regularly spaced data
 * - Real-time applications requiring fast interpolation
 * - Linear or slowly varying datasets
 * - Situations where simplicity is preferred over accuracy
 *
 * @see IInterpolationStrategy for the base interface
 * @see CubicSplineInterpolationStrategy for smooth curve alternative
 * @see BladeGeometryInterpolator for usage context
 *
 * @example
 * ```cpp
 * auto strategy = std::make_unique<LinearInterpolationStrategy>();
 * double result = strategy->interpolate(xData, yData, 2.5);
 * // Fast linear interpolation between adjacent points
 * ```
 */
class LinearInterpolationStrategy : public IInterpolationStrategy {

public:

    /**
     * @brief Performs linear interpolation between adjacent data points
     *
     * Finds the two adjacent points surrounding the target x-value and
     * performs linear interpolation to estimate the y-value. Handles
     * boundary conditions by returning endpoint values when target is
     * outside the data range.
     *
     * @param x Vector of x-coordinates (should be sorted in ascending order)
     * @param y Vector of corresponding y-values (same size as x)
     * @param targetX X-coordinate where interpolated value is desired
     * @return Linearly interpolated y-value at targetX
     * @throws std::invalid_argument if vectors have different sizes or insufficient data
     *
     * @note Returns boundary values for targets outside data range
     * @note Fast and simple but creates angular transitions at data points
     */
    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    /**
     * @brief Gets the strategy name identifier
     * @return The string "Linear"
     */
    std::string getName() const override;
};

