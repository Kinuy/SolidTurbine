#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "IInterpolationStrategy.h"

/**
 * @brief Akima spline interpolation strategy with reduced oscillations
 *
 * AkimaSplineInterpolationStrategy implements Akima's interpolation method,
 * which provides smooth curves while significantly reducing oscillations
 * and overshooting compared to standard cubic splines. Uses locally
 * adaptive derivative calculation to handle irregular data gracefully.
 *
 * ## Key Features
 * - **Oscillation Reduction**: Minimizes unwanted oscillations in irregular data
 * - **Local Adaptation**: Derivatives computed from local slope variations
 * - **Outlier Resistance**: Less sensitive to outliers than cubic splines
 * - **Smooth Curves**: Maintains C¹ continuity for smooth interpolation
 *
 * ## Algorithm Characteristics
 * - Uses weighted average of adjacent slopes for derivative calculation
 * - Weights based on absolute differences between slopes
 * - Boundary extension using slope extrapolation
 * - Hermite interpolation with Akima-computed derivatives
 *
 * ## Advantages over Cubic Splines
 * - Better behavior with non-uniform data spacing
 * - Reduced sensitivity to isolated data points
 * - More intuitive interpolation for engineering data
 * - Maintains smoothness without excessive oscillations
 *
 * ## Best Use Cases
 * - Aerodynamic data with measurement noise or irregularities
 * - Engineering datasets with local variations
 * - Performance curves with non-uniform characteristics
 * - Data where oscillation artifacts would be problematic
 *
 * @see IInterpolationStrategy for the base interface
 * @see CubicSplineInterpolationStrategy for standard cubic spline alternative
 * @see MonotonicCubicInterpolationStrategy for monotonicity-preserving option
 *
 * @example
 * ```cpp
 * auto strategy = std::make_unique<AkimaSplineInterpolationStrategy>();
 * double result = strategy->interpolate(xData, noisyYData, 2.5);
 * // Smooth interpolation with reduced oscillations from noise
 * ```
 */
class AkimaSplineInterpolationStrategy : public IInterpolationStrategy {

private:

    /**
     * @brief Calculates slope between two points
     * @param x1,y1 First point coordinates
     * @param x2,y2 Second point coordinates
     * @return Slope (y2-y1)/(x2-x1)
     */
    double calculateSlope(double x1, double y1, double x2, double y2) const;

    /**
     * @brief Calculates Akima-weighted derivative to reduce oscillations
     *
     * Uses Akima's weighting scheme to compute derivatives that are less
     * sensitive to outliers than standard cubic splines. Weights are based
     * on differences between adjacent slopes.
     *
     * @param s1,s2,s3,s4 Four consecutive slopes around the point
     * @return Weighted derivative value
     * @note Returns average of middle slopes when weights are very small
     */
    double akimaWeight(double s1, double s2, double s3, double s4) const;

public:

    /**
     * @brief Performs Akima spline interpolation with reduced oscillations
     *
     * Implements Akima's method using weighted derivatives to minimize
     * oscillations in the interpolated curve. Extends slope data at boundaries
     * and applies Hermite interpolation with Akima-computed derivatives.
     *
     * @param x Vector of x-coordinates (should be sorted)
     * @param y Vector of corresponding y-values
     * @param targetX X-coordinate for interpolation
     * @return Interpolated y-value with reduced oscillation artifacts
     * @throws std::invalid_argument if insufficient data points or size mismatch
     *
     * @note Better than cubic splines for data with local variations or outliers
     */
    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    /**
     * @brief Gets the strategy name identifier
     * @return The string "AkimaSpline"
     */
    std::string getName() const override;

};

