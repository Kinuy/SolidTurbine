#pragma once

#include <cmath>

//EnvironmentalConditions - responsible for storing environmental conditions data
struct EnvironmentalConditions {
	double airDensity;   // Density of air in kg/m^3
	double temperature;  // Temperature in Kelvin
	double atmosphericPressure;     // Pressure in Pascals
	double speedOfSound; // Speed of sound in m/s
	double gravity; // Gravitational acceleration in m/s^2
	double specificGasConstant; // Specific gas constant for dry air in J/(kg·K)
	double dynamicViscosity; // Dynamic viscosity in Pa·s
	double kinematicViscosity; // Kinematic viscosity in m^2/s

	EnvironmentalConditions(
		const double airDensity = 1.224978, 
		const double temperature = 288.15, 
		const double atmosphericPressure = 101325,
		const double speedOfSound = 340.29, 
		const double gravity = 9.81, 
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

	// Calculate dyn viscosity based on given temperature
	void calcDynamicViscosity(){
		double C_air = 120;
		double T0 = 291.15;
		double mu0 = 0.00001827;

		dynamicViscosity =  mu0 * (T0 + C_air) / (temperature + C_air) * std::pow((temperature / T0), (3 / 2));
	};
	
	// Calculate kinematic viscosity based on given dynamic viscosity and air density
	void calcKinematicViscosity(){
		kinematicViscosity = dynamicViscosity / airDensity;
	}
	

};
