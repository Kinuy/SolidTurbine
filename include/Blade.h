#pragma once

#include <vector>

// Class is responsable for all relevant data of a blade
class Blade
{
private:

	int numberOfSections;
	std::vector<double> absRadii;
	std::vector<double> relRadii;

public:

	Blade(const int numberOfSections, std::vector<double> absRadii, std::vector<double> relRadii);

	int getNumberOfSections() const;

	std::vector<double> getAbsRadii() const;

	std::vector<double> getRelRadii() const;
};

