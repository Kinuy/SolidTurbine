#pragma once

//AirfoilAeroCharacteristics - responsible for storing aerodynamic characteristics data of an airfoil
struct AirfoilAeroCharacteristic{
	double liftCoefficient;   // Coefficient of lift
	double dragCoefficient;   // Coefficient of drag
	double momentCoefficient; // Coefficient of moment
	double angleOfAttack;     // Angle of attack in degrees

	AirfoilAeroCharacteristic(double lift, double drag, double moment, double aoa)
		: liftCoefficient(lift), dragCoefficient(drag), momentCoefficient(moment), angleOfAttack(aoa) {}
};;