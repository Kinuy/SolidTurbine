#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include "IInterpolationStrategy.h"


/**
 * @brief Cubic spline interpolation strategy implementation for smooth curve fitting
 *
 * CubicSplineInterpolationStrategy provides high-quality smooth interpolation using
 * natural cubic splines. This method creates a piecewise cubic polynomial function
 * that passes through all data points while maintaining continuous first and second
 * derivatives at each connection point, resulting in visually smooth curves.
 *
 * ## Mathematical Foundation
 *
 * A cubic spline is a piecewise function where each segment between consecutive
 * data points is represented by a cubic polynomial:
 *
 * S_j(x) = a_j + b_j(x-x_j) + c_j(x-x_j)² + d_j(x-x_j)³
 *
 * For x ∈ [x_j, x_{j+1}], where j = 0, 1, ..., n-2 for n data points.
 *
 * ## Continuity Conditions
 *
 * The spline satisfies:
 * - **C⁰ continuity**: Function values match at connection points
 * - **C¹ continuity**: First derivatives match at connection points
 * - **C² continuity**: Second derivatives match at connection points
 * - **Natural boundary**: Second derivatives are zero at endpoints
 *
 * ## Algorithm Characteristics
 *
 * - **Smoothness**: Produces visually pleasing, smooth curves without oscillations
 * - **Global Method**: Each segment's coefficients depend on all data points
 * - **Computational Efficiency**: O(n) solution using tridiagonal matrix solver
 * - **Numerical Stability**: Well-conditioned for most practical datasets
 * - **Extrapolation**: Provides reasonable estimates beyond data boundaries
 *
 * ## Use Cases
 *
 * Cubic splines are ideal for:
 * - Aerodynamic data interpolation (lift/drag curves, pressure distributions)
 * - Smooth trajectory generation and motion planning
 * - Scientific data visualization requiring smooth curves
 * - Signal processing applications requiring derivative continuity
 * - Engineering applications where smoothness is critical
 *
 * ## Limitations
 *
 * - **Minimum Points**: Requires at least 3 data points for meaningful interpolation
 * - **Overshooting**: May produce values outside the range of input data
 * - **Global Sensitivity**: Adding/removing points affects the entire spline
 * - **Computational Cost**: More expensive than linear or polynomial interpolation
 *
 * @see IInterpolationStrategy for the base strategy interface
 * @see LinearInterpolationStrategy for simpler linear interpolation alternative
 * @see PolynomialInterpolationStrategy for global polynomial fitting
 *
 * @note This implementation uses natural spline boundary conditions (zero second
 *       derivatives at endpoints) which provides good behavior for most applications
 *       without requiring additional boundary condition specifications.
 *
 * @warning Input x-coordinates must be sorted in strictly ascending order.
 *          Duplicate or unsorted x-values will cause undefined behavior or
 *          mathematical errors in the coefficient calculation.
 *
 * @example
 * ```cpp
 * // Create spline interpolation strategy
 * CubicSplineInterpolationStrategy spline;
 *
 * // Data points for a smooth function
 * std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
 * std::vector<double> y = {0.0, 1.0, 8.0, 27.0, 64.0}; // y = x³
 *
 * // Interpolate at intermediate points
 * double y_15 = spline.interpolate(x, y, 1.5); // Smooth interpolation
 * double y_25 = spline.interpolate(x, y, 2.5); // Between known points
 *
 * // Strategy identification
 * std::string name = spline.getName(); // Returns "CubicSpline"
 * ```
 *
 * @example
 * ```cpp
 * // Handling extrapolation beyond data range
 * std::vector<double> x = {1.0, 2.0, 3.0};
 * std::vector<double> y = {2.0, 4.0, 6.0};
 *
 * CubicSplineInterpolationStrategy spline;
 *
 * // Interpolation within range
 * double inner = spline.interpolate(x, y, 2.5); // Cubic spline evaluation
 *
 * // Extrapolation beyond range
 * double below = spline.interpolate(x, y, 0.5); // Linear extrapolation
 * double above = spline.interpolate(x, y, 4.0); // Cubic extrapolation
 * ```
 */
class CubicSplineInterpolationStrategy : public IInterpolationStrategy {

private:

    /**
     * @brief Data structure representing a single cubic polynomial segment of the spline
     *
     * Each SplineSegment contains the mathematical representation of one piece of the
     * piecewise cubic spline function. The segment is valid over a specific x-interval
     * and stores the four coefficients needed to evaluate the cubic polynomial within
     * that interval.
     *
     * ## Mathematical Representation
     *
     * The polynomial for this segment is evaluated as:
     * f(x) = a + b*(x-x1) + c*(x-x1)² + d*(x-x1)³
     *
     * Where (x-x1) represents the local coordinate within the segment, making the
     * polynomial evaluation numerically stable and mathematically meaningful.
     *
     * ## Coefficient Interpretation
     *
     * - **a**: Function value at the left endpoint (x1). Equal to y[j] for segment j.
     * - **b**: First derivative coefficient. Related to the slope at x1.
     * - **c**: Second derivative coefficient divided by 2. Controls curvature.
     * - **d**: Third derivative coefficient divided by 6. Controls rate of curvature change.
     *
     * ## Domain Definition
     *
     * Each segment is valid only within its specified interval [x1, x2]. Attempting
     * to evaluate the polynomial outside this range may produce mathematically
     * correct but contextually meaningless results.
     *
     * @note The coefficients are calculated during spline construction to ensure
     *       C⁰, C¹, and C² continuity at segment boundaries, creating a globally
     *       smooth interpolating function.
     *
     * @see calculateSplineCoefficients() for the algorithm that computes these values
     * @see interpolate() for how these coefficients are used in evaluation
     */
    struct SplineSegment {

        /**
         * @brief Constant coefficient (function value at left endpoint)
         *
         * Represents the y-value of the spline function at x1. This coefficient
         * ensures that the polynomial passes through the data point at the
         * segment's left boundary.
         */
        double a;
        
        /**
         * @brief Linear coefficient (first derivative component)
         *
         * Controls the linear term of the polynomial. Related to the first
         * derivative of the spline at x1, influencing the initial slope
         * of the curve segment.
         */
        double b;

        /**
         * @brief Quadratic coefficient (second derivative component)
         *
         * Controls the quadratic term of the polynomial. Equal to half the
         * second derivative at x1, determining the curvature characteristics
         * of the segment.
         */
        double c;
        
        /**
         * @brief Cubic coefficient (third derivative component)
         *
         * Controls the cubic term of the polynomial. Equal to one-sixth the
         * third derivative, determining how rapidly the curvature changes
         * across the segment.
         */
        double d;

        /**
         * @brief Left boundary of the segment's valid domain
         *
         * The minimum x-value for which this polynomial segment provides
         * the correct spline evaluation. Corresponds to x[j] for segment j.
         */
        double x1;

        /**
         * @brief Right boundary of the segment's valid domain
         *
         * The maximum x-value for which this polynomial segment provides
         * the correct spline evaluation. Corresponds to x[j+1] for segment j.
         */
        double x2;
    };

    /**
     * @brief Calculates cubic spline coefficients using the natural spline approach
     *
     * Computes the coefficients for piecewise cubic polynomials that form a smooth
     * interpolating curve through the given data points. The method implements the
     * natural cubic spline algorithm, which sets the second derivatives at the
     * endpoints to zero, resulting in a "relaxed" spline that minimizes curvature.
     *
     * ## Algorithm Overview
     *
     * The natural cubic spline algorithm follows these mathematical steps:
     * 1. **Step Size Calculation**: Compute h[i] = x[i+1] - x[i] for each interval
     * 2. **Alpha Calculation**: Compute finite difference approximations for derivatives
     * 3. **Tridiagonal System**: Solve for second derivatives using Thomas algorithm
     * 4. **Coefficient Calculation**: Derive a, b, c, d coefficients for each segment
     *
     * ## Mathematical Foundation
     *
     * Each spline segment j is represented as:
     * S_j(x) = a_j + b_j(x-x_j) + c_j(x-x_j)² + d_j(x-x_j)³
     *
     * Where:
     * - a_j = y_j (function value at left endpoint)
     * - b_j = first derivative coefficient
     * - c_j = second derivative coefficient / 2
     * - d_j = third derivative coefficient / 6
     *
     * ## Natural Spline Boundary Conditions
     *
     * The natural spline enforces S''(x₀) = 0 and S''(x_n) = 0, meaning the
     * curvature is zero at both endpoints, creating a more "natural" curve shape.
     *
     * @param x Vector of x-coordinates in strictly increasing order. Must contain
     *          at least 3 points for cubic spline construction.
     * @param y Vector of corresponding y-coordinates. Must have the same size as x.
     *
     * @return Vector of SplineSegment objects, each containing coefficients (a,b,c,d)
     *         and domain interval [x1,x2] for one cubic polynomial segment.
     *         Returns (n-1) segments for n input points.
     *
     * @throws std::invalid_argument if fewer than 3 points are provided, as cubic
     *         splines require at least 3 points to define meaningful curvature
     *
     * @note The input x-coordinates are assumed to be sorted in ascending order.
     *       Unsorted input will produce mathematically incorrect results.
     *
     * @note This method performs O(n) operations using the Thomas algorithm for
     *       solving the tridiagonal system, making it efficient for large datasets.
     *
     * @see SplineSegment for the structure containing polynomial coefficients
     * @see interpolate() for using these coefficients to evaluate the spline
     *
     * @warning The algorithm assumes no duplicate x-values. Duplicate x-coordinates
     *          will cause division by zero in step size calculations.
     */
    std::vector<SplineSegment> calculateSplineCoefficients(const std::vector<double>& x, const std::vector<double>& y) const;

public:

    /**
     * @brief Performs cubic spline interpolation to estimate values at arbitrary points
     *
     * Evaluates the cubic spline function at a target x-coordinate by first computing
     * the spline coefficients, locating the appropriate polynomial segment, and then
     * evaluating the cubic polynomial at the target point. For points outside the
     * data range, the method performs linear extrapolation to provide reasonable
     * estimates beyond the known data bounds.
     *
     * ## Interpolation Process
     *
     * 1. **Validation**: Ensures input data has matching sizes and sufficient points
     * 2. **Coefficient Calculation**: Computes cubic spline coefficients for all segments
     * 3. **Segment Location**: Finds the polynomial segment containing the target x
     * 4. **Polynomial Evaluation**: Evaluates S_j(x) = a + b(x-x₁) + c(x-x₁)² + d(x-x₁)³
     *
     * ## Extrapolation Behavior
     *
     * - **Below Range** (targetX < x[0]): Uses linear extrapolation with the slope
     *   of the first spline segment at its left endpoint
     * - **Above Range** (targetX > x[n-1]): Uses the full cubic polynomial evaluation
     *   of the last segment extended beyond its right endpoint
     * - **Within Range**: Uses exact cubic spline evaluation from the appropriate segment
     *
     * @param x Vector of x-coordinates in strictly increasing order. Must contain
     *          at least 3 points for meaningful cubic spline interpolation.
     * @param y Vector of corresponding y-coordinates. Must have exactly the same
     *          size as the x vector.
     * @param targetX The x-coordinate at which to evaluate the interpolated function.
     *                Can be inside or outside the range of input x-coordinates.
     *
     * @return The interpolated or extrapolated y-value at the target x-coordinate
     *
     * @throws std::invalid_argument if x and y vectors have different sizes, or if
     *         fewer than 3 points are provided (insufficient for cubic splines)
     *
     * @note For best results, ensure x-coordinates are sorted in ascending order.
     *       The method does not verify or enforce this sorting requirement.
     *
     * @note Extrapolation accuracy decreases rapidly outside the data range.
     *       Linear extrapolation below the range provides more conservative estimates
     *       than cubic extrapolation above the range.
     *
     * @warning The cubic spline may exhibit oscillatory behavior if the input data
     *          contains noise or sharp changes. Consider data smoothing for noisy datasets.
     *
     * @see calculateSplineCoefficients() for the underlying coefficient computation
     * @see SplineSegment for the polynomial representation used in evaluation
     *
     * @example
     * ```cpp
     * std::vector<double> x = {1.0, 2.0, 3.0, 4.0, 5.0};
     * std::vector<double> y = {1.0, 4.0, 9.0, 16.0, 25.0}; // y = x²
     *
     * CubicSplineInterpolationStrategy spline;
     * double result = spline.interpolate(x, y, 2.5); // Interpolate at x=2.5
     * // result ≈ 6.25 (close to 2.5² = 6.25)
     * ```
     */
    double interpolate(const std::vector<double>& x, const std::vector<double>& y, double targetX) const override;

    /**
     * @brief Gets the name identifier for this interpolation strategy
     *
     * Returns a string identifier that uniquely identifies this interpolation
     * method within a strategy pattern system. This name is typically used
     * for strategy registration, user interface display, logging, and
     * configuration file identification.
     *
     * @return The string "CubicSpline" identifying this interpolation strategy
     *
     * @note This identifier should remain constant across different versions
     *       to maintain configuration file compatibility and API stability.
     *
     * @see IInterpolationStrategy for the base strategy interface
     */
    std::string getName() const override;

};

