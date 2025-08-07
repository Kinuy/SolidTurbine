#pragma once

#include "ITurbine.h"

// Concrete horizontal turbine implementation
class HorizontalTurbine : public ITurbine
{
private:

	int numberOfBlades; // Number of blades in the turbine

public:

	HorizontalTurbine(const int numberOfBlades);

	// Override the getNumberOfBlades method from ITurbine interface
	int getNumberOfBlades() const override;
};

