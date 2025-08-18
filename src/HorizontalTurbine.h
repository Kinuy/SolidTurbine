#pragma once

#include "ITurbine.h"

/**
 * @brief Horizontal axis turbine implementation with configurable blade count
 *
 * HorizontalTurbine implements ITurbine interface for horizontal axis wind
 * turbines, the most common commercial wind turbine design. Provides basic
 * turbine configuration with user-specified blade count.
 *
 * ## Key Features
 * - **Configurable Blade Count**: Supports various blade configurations
 * - **Industry Standard**: Models typical horizontal axis wind turbines
 * - **Interface Compliance**: Implements ITurbine for polymorphic usage
 *
 * ## Typical Configurations
 * - **3 Blades**: Most common for modern large wind turbines
 * - **2 Blades**: Some smaller or older turbine designs
 * - **Variable**: Supports experimental or specialized configurations
 *
 * @see ITurbine for the base interface
 *
 * @example
 * ```cpp
 * HorizontalTurbine turbine(3);  // Standard 3-blade configuration
 * int blades = turbine.getNumberOfBlades();
 *
 * // Polymorphic usage
 * std::unique_ptr<ITurbine> turbine = std::make_unique<HorizontalTurbine>(2);
 * ```
 */
class HorizontalTurbine : public ITurbine
{
private:

	/**
	 * @brief Number of blades configured for this turbine
	 */
	int numberOfBlades; // Number of blades in the turbine

public:

	/**
	 * @brief Constructor initializing horizontal turbine with specified blade count
	 * @param numberOfBlades Number of blades for the turbine (default: 3)
	 * @note Default of 3 blades is typical for modern horizontal axis wind turbines
	 */
	HorizontalTurbine(const int numberOfBlades);

	/**
	 * @brief Gets the number of blades for this turbine
	 * @return Number of blades as configured during construction
	 * @note Implements ITurbine interface requirement
	 */
	int getNumberOfBlades() const override;
};

