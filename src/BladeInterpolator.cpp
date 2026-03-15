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
    const std::vector<double> radiusValues = bladeGeometry->getRadiusValues();
    const int n = static_cast<int>(radiusValues.size());

    // Pre-allocate so each thread writes to its own slot — no push_back race.
    bladeSections.resize(static_cast<std::size_t>(n));

    // Thread-safety rationale:
    //  • bladeSections[i] — each thread owns one slot; no aliasing.
    //  • bladeGeometry->getRowByRadius() / getRadiusValues() — read-only data.
    //  • AirfoilPolarInterpolationFactory / AirfoilGeometryInterpolationFactory
    //    — called with section-local inputs; assumed stateless (factory pattern).
    //  • All intermediate objects (bladeSection, airfoilPolar, airfoilGeometry)
    //    are stack-local unique_ptrs; no sharing between threads.
    #pragma omp parallel for schedule(dynamic, 1) default(none) \
        shared(radiusValues, n)
    for (int i = 0; i < n; ++i)
        interpolateSection(static_cast<std::size_t>(i), radiusValues[static_cast<std::size_t>(i)]);

    std::cout << "\nSuccessfully interpolated " << bladeSections.size()
              << " sections geo and perfo data" << std::endl;
}

void BladeInterpolator::interpolateSection(std::size_t index, double targetRadius)
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

	// Scale airfoil geometry with chord and max thickness
	airfoilGeometry->applyScalingWithChordAndMaxThickness(bladeSection->chord, bladeSection->relativeThickness / 100.0 * bladeSection->chord, targetRadius);

	// Translate pitch axis to origin so the section rotates in place around
	// the pitch axis. Without this step the twist rotation introduces a net
	// x-offset and sections with different chords / pitch axes are not aligned
	// on a common blade reference line.
	airfoilGeometry->applyPitchAxisToOrigin(bladeSection->relativeTwistAxis);

	// Apply twist angle around quarter chord + relative twist axis position point
	airfoilGeometry->applyTwistAroundQuarterChord(bladeSection->twist, bladeSection->relativeTwistAxis);

	// Apply prebend PCBAx and sweep PCBAy 
	airfoilGeometry->applyTranslationXY(bladeSection->pcbaX, bladeSection->pcbaY);

	// Set the section properties
	bladeSection->airfoilPolar = std::move(airfoilPolar);
	bladeSection->airfoilGeometry = std::move(airfoilGeometry);

    // Write directly into the pre-allocated slot — no mutex needed.
    bladeSections[index] = std::move(bladeSection);
}

const std::vector<std::unique_ptr<BladeGeometrySection>> &BladeInterpolator::getBladeSections() const
{
	return bladeSections;
}
