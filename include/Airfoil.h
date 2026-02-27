#pragma once

#include <string>
#include <vector>

#include "AirfoilAeroCharacteristic.h"

/**
 * @brief Airfoil data class containing geometric and aerodynamic properties
 *
 * Airfoil represents a complete airfoil definition including geometric parameters,
 * operational features, design classification, and associated aerodynamic
 * characteristics. Used for airfoil database management and aerodynamic analysis.
 *
 * ## Key Features
 * - **Geometric Properties**: Thickness ratios and dimensional characteristics
 * - **Operational Features**: Vortex generator presence and configuration
 * - **Design Classification**: Custom vs standard airfoil designation
 * - **Aerodynamic Data**: Associated performance characteristics and coefficients
 *
 * ## Use Cases
 * - Airfoil database and catalog management
 * - Blade design and airfoil selection processes
 * - Aerodynamic analysis with airfoil-specific data
 * - CFD setup and validation with airfoil properties
 *
 * @see AirfoilAeroCharacteristic for aerodynamic data structure
 * @see AirfoilGeometryData for coordinate-based airfoil representation
 *
 * @example
 * ```cpp
 * Airfoil naca0012("NACA0012", 0.12, 12.0, false, false, aeroData);
 * std::string name = naca0012.getName();
 * double thickness = naca0012.getRelThicknessPercent();
 * ```
 */
class Airfoil{

private:
	
	/**
	 * @brief Airfoil identifier/designation (e.g., "NACA0012", "S809")
	 */
	std::string name;

	/**
	 * @brief Relative thickness as decimal fraction (e.g., 0.12 for 12%)
	 */
	double relThickness;

	/**
	 * @brief Relative thickness as percentage (e.g., 12.0 for 12%)
	 */
	double relThicknessPercent;

	/**
	 * @brief Whether airfoil has vortex generators for flow control
	 */
	bool hasVG;

	/**
	 * @brief Whether this is a custom/design airfoil vs standard catalog airfoil
	 */
	bool isDesignAirfoil;

	/**
	 * @brief Collection of aerodynamic characteristic data for different conditions
	 */
	std::vector<AirfoilAeroCharacteristic*> airfoilAeroCharacteristics;

public:

	/**
	 * @brief Constructor initializing airfoil with geometric and aerodynamic properties
	 * @param name Airfoil identifier/designation (default: empty string)
	 * @param relThickness Relative thickness as decimal fraction (default: 0.0)
	 * @param relThicknessPercent Relative thickness as percentage (default: 0.0)
	 * @param hasVG Whether airfoil has vortex generators (default: false)
	 * @param isDesignAirfoil Whether this is a design/custom airfoil (default: false)
	 * @param airfoilAeroCharacteristics Vector of aerodynamic characteristic data (default: empty)
	 */
	Airfoil(
		const std::string& name,
		const double relThickness,
		const double relThicknessPercent,
		const bool hasVG,
		const bool isDesignAirfoil,
		const std::vector<AirfoilAeroCharacteristic*> airfoilAeroCharacteristics
	);

	/**
	 * @brief Gets the airfoil name/identifier
	 * @return Airfoil name string
	 * @note Current implementation returns empty string - should return stored name
	 */
	std::string getName() const;

	/**
	 * @brief Gets the relative thickness as decimal fraction
	 * @return Relative thickness value
	 * @note Current implementation returns 0.0 - should return stored relThickness
	 */
	double getRelThickness() const;

	/**
	 * @brief Gets the relative thickness as percentage
	 * @return Relative thickness percentage value
	 * @note Current implementation returns 0.0 - should return stored relThicknessPercent
	 */
	double getRelThicknessPercent() const;

	/**
	 * @brief Gets whether airfoil has vortex generators
	 * @return true if airfoil has vortex generators, false otherwise
	 * @note Current implementation returns false - should return stored hasVG
	 */
	bool getHasVG() const;

	/**
	 * @brief Gets whether this is a design/custom airfoil
	 * @return true if design airfoil, false otherwise
	 * @note Current implementation returns false - should return stored isDesignAirfoil
	 */
	bool getIsDesignAirfoil() const;

	/**
	 * @brief Gets the aerodynamic characteristics data
	 * @return Vector of pointers to AirfoilAeroCharacteristic objects
	 * @note Current implementation returns empty vector - should return stored characteristics
	 */
	std::vector<AirfoilAeroCharacteristic*> getAirfoilAeroCharacteristics() const;

};

