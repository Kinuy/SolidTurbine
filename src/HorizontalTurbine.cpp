#include "HorizontalTurbine.h"

HorizontalTurbine::HorizontalTurbine(std::unique_ptr<BladeInterpolator>) : numberOfBlades_(3)
{
}

// Override the getNumberOfBlades method from ITurbine interface
int HorizontalTurbine::getNumberOfBlades() const{
	return numberOfBlades_;
}