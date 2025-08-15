#pragma once

#include <string>
#include <vector>

#include "AirfoilAeroCharacteristic.h"

// Airfoil class is responsible for storing all relevant data of an airfoil
class Airfoil
{
private:
	
	std::string name;
	double relThickness;
	double relThicknessPercent;
	bool hasVG;
	bool isDesignAirfoil;
	std::vector<AirfoilAeroCharacteristic*> airfoilAeroCharacteristics;

public:

	Airfoil(
		const std::string& name,
		const double relThickness,
		const double relThicknessPercent,
		const bool hasVG,
		const bool isDesignAirfoil,
		const std::vector<AirfoilAeroCharacteristic*> airfoilAeroCharacteristics
	);

	std::string getName() const;

	double getRelThickness() const;

	double getRelThicknessPercent() const;

	bool getHasVG() const;

	bool getIsDesignAirfoil() const;

	std::vector<AirfoilAeroCharacteristic*> getAirfoilAeroCharacteristics() const;


};

