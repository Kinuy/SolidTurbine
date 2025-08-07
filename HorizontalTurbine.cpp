#include "HorizontalTurbine.h"

HorizontalTurbine::HorizontalTurbine(const int blades = 3) 
	: 
	numberOfBlades(blades) {
}

// Override the getNumberOfBlades method from ITurbine interface
int HorizontalTurbine::getNumberOfBlades() const{
	return numberOfBlades;
}