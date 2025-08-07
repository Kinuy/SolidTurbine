#include "Blade.h"

Blade::Blade(const int numsec = 0, std::vector<double> absRadii, std::vector<double> relRadii)
	: 
	numberOfSections(numsec),
	absRadii(absRadii),
	relRadii(relRadii)
{
	numberOfSections = absRadii.size();
}

int Blade::getNumberOfSections() const
{
	return numberOfSections;
}

std::vector<double> Blade::getAbsRadii() const
{
	return absRadii;
}

std::vector<double> Blade::getRelRadii() const
{
	return relRadii;
}
