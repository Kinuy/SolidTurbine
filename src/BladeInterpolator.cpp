#include "BladeInterpolator.h"
#include <iostream>


BladeInterpolator::BladeInterpolator(
	const BladeGeometryData* bladeGeoSections,
	const std::vector<const AirfoilGeometryData*>& airfoilGeoms, 
	const std::vector<const AirfoilPolarData*>& airfoilPerfos) 
	:
	bladeGeometry(bladeGeoSections),
	airfoilGeometries(airfoilGeoms),
	airfoilPerformances(airfoilPerfos)
{
	interpolateAllSections();
}

void BladeInterpolator::interpolateAllSections()
{
	// Get all radius values from the blade geometry
	std::vector<double>  radiusValues = bladeGeometry->getRadiusValues();
	// Interpolate each section based on the radius values
	for (const auto& radius : radiusValues) {
		interpolateSection(radius);
	}
	std::cout << "\nSuccessfully interpolated " << bladeSections.size() << " sections geo and perfo data" << std::endl;
}

void BladeInterpolator::interpolateSection(double targetRadius)
{
	// Create a section object and fill it with data: blade section geo, airfoil geo , airfoil perfo 
	BladeGeometryData* mutableBladeGeom = const_cast<BladeGeometryData*>(bladeGeometry);
	BladeGeometrySection existingSection = mutableBladeGeom->getRowByRadius(targetRadius);

	// Get section data
	std::unique_ptr<BladeGeometrySection> bladeSection = std::make_unique<BladeGeometrySection>(existingSection);

	// Get airfoil polar data
	std::unique_ptr<AirfoilPolarData> airfoilPolar = AirfoilPolarInterpolationFactory::getPolarForSection(airfoilPerformances, bladeSection->relativeThickness);
	
	// Get airfoil geo data
	std::unique_ptr<AirfoilGeometryData> airfoilGeometry = AirfoilGeometryInterpolationFactory::getAirfoilGeometryForSection(airfoilGeometries, bladeSection->relativeThickness);

	// Set z-coordinate to section relative radius
	double lastBladeRadius = bladeGeometry->getRadiusValues().back();
	airfoilGeometry->setZCoordinates(targetRadius / lastBladeRadius);

	// Set the section properties
	bladeSection->airfoilPolar = std::move(airfoilPolar);
	bladeSection->airfoilGeometry = std::move(airfoilGeometry);


    // Create and store the section
    bladeSections.push_back(std::move(bladeSection));

}

std::vector<std::unique_ptr<BladeGeometrySection>> BladeInterpolator::getBladeSections()
{
	return std::move(bladeSections);
}
