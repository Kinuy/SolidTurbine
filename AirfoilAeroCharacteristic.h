#pragma once

#include <vector>

#include "AeroCharacteristicPoint.h"

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

