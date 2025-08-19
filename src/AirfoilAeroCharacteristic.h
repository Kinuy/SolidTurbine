#pragma once

#include <vector>

#include "AeroCharacteristicPoint.h"

/**
 * @brief Aerodynamic characteristic data container
 *
 * @deprecated This class is deprecated. Use AirfoilPerformanceData instead
 * for improved design, better performance, and enhanced functionality.
 *
 * AirfoilAeroCharacteristic was the original class for storing airfoil
 * aerodynamic data but has been replaced by AirfoilPerformanceData which
 * provides better structure, improved parsing capabilities, and more
 * comprehensive analysis methods.
 *
 * ## Migration Guide
 * - Replace AirfoilAeroCharacteristic with AirfoilPerformanceData
 * - Update data point usage to AirfoilPerformancePoint
 * - Use new parser classes for improved file handling
 *
 * @see AirfoilPerformanceData for the recommended replacement
 * @see AirfoilPerformancePoint for individual data points
 * @see AirfoilPerformanceParser for improved file parsing
 *
 * @example
 * ```cpp
 * // Deprecated usage:
 * AirfoilAeroCharacteristic* oldCharacteristic; // DON'T USE
 *
 * // Use instead:
 * auto newPerformanceData = std::make_unique<AirfoilPerformanceData>();
 * newPerformanceData->addPerformancePoint(5.0, 0.8, 0.02, -0.1);
 * ```
 */
// TODO: Remove this class in future versions
class AirfoilAeroCharacteristic
{
private:
	double reynoldsNumber;
	double machNumber;
	std::vector<AeroCharacteristicPoint*> aeroCharacteristic;

public:

	AirfoilAeroCharacteristic();
	double getReynoldsNumber() const;
	double getMachNumber() const;
	std::vector<AeroCharacteristicPoint*> getAeroCharacteristic() const;

};

