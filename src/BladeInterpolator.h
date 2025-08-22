#pragma once

#include <vector>
#include <memory>
#include "AirfoilGeometryData.h"
#include "AirfoilPolarData.h"
#include "BladeGeometryData.h"

#include "BladeGeometrySection.h"
#include "AirfoilPolarInterpolationFactory.h"
#include "AirfoilGeometryInterpolationFactory.h"
/**
 * @brief Blade section data interpolation engine
 */
class BladeInterpolator
{

private:

	/**
	 * @brief Reference to blade geometry data containing radial sections
	 *
	 * Provides the base geometric data (radius, chord, twist, thickness)
	 * for all radial stations along the blade span.
	 */
	const BladeGeometryData* bladeGeometry;
	/**
	 * @brief Collection of available airfoil geometries for section matching
	 *
	 * Vector of pointers to airfoil geometry data used for selecting
	 * appropriate airfoils based on thickness criteria.
	 */
	const std::vector<const AirfoilGeometryData*>& airfoilGeometries;
	/**
	 * @brief Collection of available airfoil performance data for section matching
	 *
	 * Vector of pointers to airfoil polar data used for selecting
	 * appropriate performance based on thickness criteria.
	 */
	const std::vector<const AirfoilPolarData*>& airfoilPerformances;

	/**
	 * @brief Store for all interpolated blade sections
	 */
	std::vector<std::unique_ptr<BladeGeometrySection>> bladeSection;
	

public:

	/**
	 * @brief Interpolates all blade sections from geometry and perfo data
	 */
	BladeInterpolator(
		const BladeGeometryData* bladeGeoSections,
		const std::vector<const AirfoilGeometryData*>& airfoilGeoms,
		const std::vector<const AirfoilPolarData*>& airfoilPerfos
	);

	/**
	 * @brief Interpolates all blade sections based on the geometry data
	 * 
	 */
	void interpolateAllSections();

	/**
	 * @brief Interpolates blade section data at a specific thickness
	 * @param targetThickness Thickness at which to interpolate blade properties
	 * @return Interpolated section at target thickness
	 */
	void interpolateSection(double targetThickness);

};

