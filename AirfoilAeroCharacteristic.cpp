#include "AirfoilAeroCharacteristic.h"

AirfoilAeroCharacteristic::AirfoilAeroCharacteristic()
{
}

double AirfoilAeroCharacteristic::getReynoldsNumber() const
{
	return 0.0;
}

double AirfoilAeroCharacteristic::getMachNumber() const
{
	return 0.0;
}

std::vector<AeroCharacteristicPoint*> AirfoilAeroCharacteristic::getAeroCharacteristic() const
{
	return std::vector<AeroCharacteristicPoint*>();
}
