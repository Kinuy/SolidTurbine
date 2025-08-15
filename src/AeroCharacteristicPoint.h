#pragma once

//AeroCharacteristicPoint - responsible for storing one point of aerodynamic characteristics data of an airfoil
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