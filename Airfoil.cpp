#include "Airfoil.h"

Airfoil::Airfoil(
	const std::string& name = "",
	const double relThickness = 0.0, 
	const double relThicknessPercent = 0.0, 
	const bool hasVG = false, 
	const bool isDesignAirfoil = false, 
	const std::vector<AirfoilAeroCharacteristic*> airfoilAeroCharacteristics = {})
	:
	name(name),
	relThickness(relThickness),
	relThicknessPercent(relThicknessPercent),
	hasVG(hasVG),
	isDesignAirfoil(isDesignAirfoil),
	airfoilAeroCharacteristics(airfoilAeroCharacteristics)
{
}

std::string Airfoil::getName() const
{
	return std::string();
}

double Airfoil::getRelThickness() const
{
	return 0.0;
}

double Airfoil::getRelThicknessPercent() const
{
	return 0.0;
}

bool Airfoil::getHasVG() const
{
	return false;
}

bool Airfoil::getIsDesignAirfoil() const
{
	return false;
}

std::vector<AirfoilAeroCharacteristic*> Airfoil::getAirfoilAeroCharacteristics() const
{
	return std::vector<AirfoilAeroCharacteristic*>();
}
