#pragma once

/**
 * @brief Enumeration of available interpolation methods for data analysis
 *
 * InterpolationMethod defines the mathematical algorithms available for
 * interpolating values between known data points. Each method has different
 * characteristics regarding smoothness, computational cost, and behavior
 * with various data types.
 *
 * ## Method Characteristics
 * - **LINEAR**: Fast, simple, but may create sharp corners at data points
 * - **CUBIC_SPLINE**: Smooth curves with continuous derivatives, may oscillate
 * - **AKIMA_SPLINE**: Reduced oscillation compared to cubic splines
 * - **MONOTONIC_CUBIC_SPLINE**: Preserves monotonic trends in data
 *
 * ## Selection Guidelines
 * - Use **LINEAR** for fast approximations or when simplicity is preferred
 * - Use **CUBIC_SPLINE** for smooth curves when data is well-behaved
 * - Use **AKIMA_SPLINE** to reduce oscillations in noisy or irregular data
 * - Use **MONOTONIC_CUBIC_SPLINE** when data trends must be preserved
 *
 * @see IInterpolationStrategy for implementation interface
 * @see BladeGeometryInterpolator for usage context
 *
 * @example
 * ```cpp
 * InterpolationMethod method = InterpolationMethod::CUBIC_SPLINE;
 * auto interpolator = createBladeInterpolator(method);
 * ```
 */
enum class InterpolationMethod {

	/**
	 * @brief Linear interpolation between adjacent points
	 * Fast and simple but creates angular transitions at data points
	 */
	LINEAR,                // Linear interpolation

	/**
	 * @brief Cubic spline interpolation with smooth curves
	 * Provides C² continuity but may exhibit oscillations with irregular data
	 */
	CUBIC_SPLINE,         // Cubic spline interpolation

	/**
	 * @brief Akima spline interpolation with reduced oscillation
	 * Balances smoothness with local data behavior, reducing unwanted oscillations
	 */
	AKIMA_SPLINE,         // Akima spline interpolation

	/**
	 * @brief Monotonic cubic spline preserving data trends
	 * Maintains monotonic behavior of input data while providing smooth interpolation
	 */
	MONOTONIC_CUBIC_SPLINE       // Monotonic cubic spline interpolation

};
