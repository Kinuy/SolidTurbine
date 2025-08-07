#include "HorizontalTurbine.h"

HorizontalTurbine::HorizontalTurbine(const int numberOfBlades = 3)
	: 
	numberOfBlades(numberOfBlades) {
}

// Override the getNumberOfBlades method from ITurbine interface
int HorizontalTurbine::getNumberOfBlades() const{
	return numberOfBlades;
}