#pragma once

#include <vector>

/**
 * @brief Data structure for turbine operational conditions and parameters
 *
 * OperationConditions contains vectors of operational parameters used in
 * aerodynamic analysis and performance calculations. Supports multiple
 * operating points for comprehensive turbine performance evaluation.
 *
 * ## Parameter Relationships
 * Tip speed ratio relates wind speed to rotational speed:
 * TSR = (blade_tip_speed) / (wind_speed) = (omega * R) / (wind_speed)
 *
 * ## Typical Usage
 * Used in conjunction with EnvironmentalConditions for complete
 * aerodynamic analysis setup in simulation workflows.
 *
 * @see EnvironmentalConditions for atmospheric parameters
 * @see IAerodynamicAnalyzer for analysis interface using these conditions
 *
 * @example
 * ```cpp
 * OperationConditions ops({5.0, 10.0, 15.0},  // wind speeds
 *                        {6.0, 7.0, 8.0},    // tip speed ratios
 *                        {0.0, 2.0, 5.0});   // pitch angles
 * ```
 */
struct OperationConditions {

	/**
	 * @brief Wind speeds for analysis [m/s]
	 */
	std::vector<double> windSpeeds;	// wind speeds vector in m/s

	/**
	 * @brief Tip speed ratios (dimensionless)
	 *
	 * Ratio of blade tip speed to wind speed, typically ranging from 4-12
	 * for modern horizontal axis wind turbines.
	 */
	std::vector<double> tipSpeedRatios; // tip speed ratios vector : windSpeeds / rotationalSpeeds

	/**
	 * @brief Blade pitch angles [degrees]
	 *
	 * Collective pitch angles for blade feathering and power control.
	 * Positive values typically represent feathering toward stall.
	 */
	std::vector<double> pitchAngles; // pitch angles vector in degrees

	/**
	 * @brief Constructor initializing all operational parameter vectors
	 * @param windSpeeds Vector of wind speeds [m/s]
	 * @param tipSpeedRatios Vector of tip speed ratios (dimensionless)
	 * @param pitchAngles Vector of pitch angles [degrees]
	 */
	OperationConditions(std::vector<double> windSpeeds , std::vector<double> tipSpeedRatios , std::vector<double> pitchAngles):
		windSpeeds(windSpeeds), 
		tipSpeedRatios(tipSpeedRatios), 
		pitchAngles(pitchAngles) {
	}

};
