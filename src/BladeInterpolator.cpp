#include "BladeInterpolator.h"


BladeInterpolator::BladeInterpolator(
	const BladeGeometryData* bladeGeoSections,
	const std::vector<const AirfoilGeometryData*>& airfoilGeoms, 
	const std::vector<const AirfoilPolarData*>& airfoilPerfos) 
	:
	bladeGeometry(bladeGeoSections),
	airfoilGeometries(airfoilGeoms),
	airfoilPerformances(airfoilPerfos)
{
}

void BladeInterpolator::interpolateAllSections()
{
	// Get all radius values from the blade geometry
	auto radiusValues = bladeGeometry->getRadiusValues();
	// Interpolate each section based on the radius values
	for (const auto& radius : radiusValues) {
		interpolateSection(radius);
	}
}

void BladeInterpolator::interpolateSection(double targetThickness)
{
	//BladeGeometrySection section = bladeGeometry.getRowByRadius(targetThickness);
	//auto airfoilPolar = AirfoilPolarInterpolationFactory::getPolarForSection(airfoilPerformances, targetThickness);
	//auto airfoilGeometry = AirfoilGeometryInterpolationFactory::getAirfoilGeometryForSection(airfoilGeometries, targetThickness);

	//// Set the section properties
	//section.airfoilPolar = std::move(airfoilPolar);
	//section.airfoilGeometry = std::move(airfoilGeometry);


 //   // Create and store the section
 //   bladeSection.push_back(std::make_unique<BladeGeometrySection>(std::move(section)));

}
