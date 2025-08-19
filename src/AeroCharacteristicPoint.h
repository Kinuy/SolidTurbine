#pragma once

/**
 * @brief Data point for aerodynamic characteristics
 *
 * @deprecated This struct is deprecated. Use AirfoilPerformancePoint instead
 * for better naming consistency and improved functionality.
 *
 * AeroCharacteristicPoint represents a single aerodynamic data point but has
 * been superseded by AirfoilPerformancePoint which provides better naming
 * conventions and additional functionality for aerodynamic analysis.
 *
 * @see AirfoilPerformancePoint for the recommended replacement
 * @see AirfoilPerformanceData for collections of performance points
 *
 * @example
 * ```cpp
 * // Deprecated usage:
 * AeroCharacteristicPoint oldPoint; // DON'T USE
 *
 * // Use instead:
 * AirfoilPerformancePoint newPoint(5.0, 0.8, 0.02, -0.1);
 * ```
 */
 // TODO: Remove in future versions
struct AeroCharacteristicPoint {
	double liftCoefficient;   // Coefficient of lift
	double dragCoefficient;   // Coefficient of drag
	double momentCoefficient; // Coefficient of moment
	double angleOfAttack;     // Angle of attack in degrees

	AeroCharacteristicPoint(
		const double liftCoefficient, 
		const double dragCoefficient, 
		const double momentCoefficient, 
		const double angleOfAttack)
		: 
		liftCoefficient(liftCoefficient), 
		dragCoefficient(dragCoefficient), 
		momentCoefficient(momentCoefficient), 
		angleOfAttack(angleOfAttack) {
	}
};