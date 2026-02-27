#pragma once

#include <cmath>

/**
 * @brief Structure representing environmental and atmospheric conditions for aerodynamic calculations
 *
 * This structure encapsulates all relevant environmental parameters needed for
 * aerodynamic simulations and calculations. It provides default values corresponding
 * to standard atmospheric conditions at sea level (15°C, 1 atm) and includes
 * automatic calculation of derived properties like viscosities.
 *
 * The structure automatically computes dynamic and kinematic viscosity based on
 * temperature and air density using established atmospheric models.
 */
struct EnvironmentalConditions {

	/**
	 * @brief Density of air in kg/m³
	 *
	 * Air density affects lift, drag, and other aerodynamic forces.
	 * Default value: 1.224978 kg/m³ (sea level, 15°C)
	 */
	double airDensity;   // Density of air in kg/m^3

	/**
	 * @brief Temperature in Kelvin
	 *
	 * Temperature affects air density, viscosity, and speed of sound.
	 * Default value: 288.15 K (15°C)
	 */
	double temperature;  // Temperature in Kelvin

	/**
	 * @brief Atmospheric pressure in Pascals
	 *
	 * Static atmospheric pressure at the operating altitude.
	 * Default value: 101325 Pa (sea level pressure)
	 */
	double atmosphericPressure;     // Pressure in Pascals

	/**
	 * @brief Speed of sound in m/s
	 *
	 * Used for compressibility effects and Mach number calculations.
	 * Default value: 340.29 m/s (sea level, 15°C)
	 */
	double speedOfSound; // Speed of sound in m/s

	/**
	 * @brief Gravitational acceleration in m/s²
	 *
	 * Local gravitational acceleration, varies slightly with altitude and latitude.
	 * Default value: 9.81 m/s² (standard gravity)
	 */
	double gravity; // Gravitational acceleration in m/s^2

	/**
	 * @brief Specific gas constant for dry air in J/(kg·K)
	 *
	 * Gas constant for dry air, used in ideal gas law calculations.
	 * Default value: 287.058 J/(kg·K)
	 */
	double specificGasConstant; // Specific gas constant for dry air in J/(kg·K)

	/**
	 * @brief Dynamic viscosity in Pa·s
	 *
	 * Measure of fluid's resistance to shear deformation.
	 * Calculated automatically using Sutherland's formula if not provided.
	 */
	double dynamicViscosity; // Dynamic viscosity in Pa·s

	/**
	 * @brief Kinematic viscosity in m²/s
	 *
	 * Ratio of dynamic viscosity to air density.
	 * Calculated automatically as dynamicViscosity / airDensity.
	 */
	double kinematicViscosity; // Kinematic viscosity in m^2/s

	/**
	 * @brief Constructor with default atmospheric conditions at sea level
	 *
	 * Creates an EnvironmentalConditions object with specified or default values.
	 * All default values correspond to International Standard Atmosphere (ISA)
	 * conditions at sea level and 15°C. Dynamic and kinematic viscosities are
	 * automatically calculated using established atmospheric models.
	 *
	 * @param airDensity Air density in kg/m³ (default: 1.224978 kg/m³)
	 * @param temperature Temperature in Kelvin (default: 288.15 K = 15°C)
	 * @param atmosphericPressure Atmospheric pressure in Pascals (default: 101325 Pa)
	 * @param speedOfSound Speed of sound in m/s (default: 340.29 m/s)
	 * @param gravity Gravitational acceleration in m/s² (default: 9.81 m/s²)
	 * @param specificGasConstant Specific gas constant in J/(kg·K) (default: 287.058 J/(kg·K))
	 * @param dynamicViscosity Dynamic viscosity in Pa·s (default: 0, will be calculated)
	 * @param kinematicViscosity Kinematic viscosity in m²/s (default: 0, will be calculated)
	 */
	EnvironmentalConditions(
		const double airDensity = 1.224978, // Default value at sea level and 15 degrees Celsius 
		const double temperature = 288.15, // Default value at sea level and 15 degrees Celsius 
		const double atmosphericPressure = 101325, // Default value at sea level in Pascals
		const double speedOfSound = 340.29, // Default value at sea level and 15 degrees Celsius in m/s 
		const double gravity = 9.81, // Default value for gravitational acceleration in m/s^2 
		const double specificGasConstant = 287.058,   
		const double dynamicViscosity = 0, 
		const double kinematicViscosity = 0)
		: 
		airDensity(airDensity), 
		temperature(temperature), 
		atmosphericPressure(atmosphericPressure),
		speedOfSound(speedOfSound), 
		gravity(gravity), 
		specificGasConstant(specificGasConstant),
		dynamicViscosity(dynamicViscosity), 
		kinematicViscosity(kinematicViscosity) {

		calcDynamicViscosity();
		calcKinematicViscosity();

	}

	/**
	 * @brief Calculate dynamic viscosity based on temperature using Sutherland's formula
	 *
	 * Computes dynamic viscosity using Sutherland's formula for air:
	 * μ = μ₀ × (T₀ + C)/(T + C) × (T/T₀)^(3/2)
	 *
	 * Where:
	 * - μ₀ = 1.827 × 10⁻⁵ Pa·s (reference viscosity)
	 * - T₀ = 291.15 K (reference temperature)
	 * - C = 120 K (Sutherland constant for air)
	 * - T = current temperature
	 *
	 * This method is automatically called in the constructor and updates
	 * the dynamicViscosity member variable.
	 */
	void calcDynamicViscosity(){
		double C_air = 120;
		double T0 = 291.15;
		double mu0 = 0.00001827;

		dynamicViscosity =  mu0 * (T0 + C_air) / (temperature + C_air) * std::pow((temperature / T0), (3 / 2));
	};
	
	/**
	 * @brief Calculate kinematic viscosity from dynamic viscosity and air density
	 *
	 * Computes kinematic viscosity using the relationship:
	 * ν = μ / ρ
	 *
	 * Where:
	 * - ν = kinematic viscosity (m²/s)
	 * - μ = dynamic viscosity (Pa·s)
	 * - ρ = air density (kg/m³)
	 *
	 * This method is automatically called in the constructor after
	 * calcDynamicViscosity() and updates the kinematicViscosity member variable.
	 */
	void calcKinematicViscosity(){
		kinematicViscosity = dynamicViscosity / airDensity;
	}
	

};
