#pragma once

#include <vector>
#include <memory>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "AirfoilGeometryData.h"
#include "AirfoilPolarData.h"
#include "BladeGeometryData.h"
// #include "Angles.h"

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
	const BladeGeometryData *bladeGeometry;
	/**
	 * @brief Collection of available airfoil geometries for section matching
	 *
	 * Vector of pointers to airfoil geometry data used for selecting
	 * appropriate airfoils based on thickness criteria.
	 */
	const std::vector<const AirfoilGeometryData *> airfoilGeometries;
	/**
	 * @brief Collection of available airfoil performance data for section matching
	 *
	 * Vector of pointers to airfoil polar data used for selecting
	 * appropriate performance based on thickness criteria.
	 */
	const std::vector<const AirfoilPolarData *> airfoilPerformances;

	/**
	 * @brief Store for all interpolated blade sections
	 */
	std::vector<std::unique_ptr<BladeGeometrySection>> bladeSections;

public:
	/**
	 * @brief Interpolates all blade sections from geometry and perfo data
	 */
	BladeInterpolator(
		const BladeGeometryData *bladeGeoSections,
		const std::vector<const AirfoilGeometryData *> &airfoilGeoms,
		const std::vector<const AirfoilPolarData *> &airfoilPerfos);

	/**
	 * @brief Interpolates all blade sections based on the geometry data
	 *
	 */
	void interpolateAllSections();

	/**
	 * @brief Interpolates blade section data at a specific radius and stores
	 *        the result at the pre-allocated slot @p index in bladeSections.
	 *
	 * @param index        Position in bladeSections to write into.
	 * @param targetRadius Radial position [m] at which to interpolate.
	 */
	void interpolateSection(std::size_t index, double targetRadius);

	const std::vector<std::unique_ptr<BladeGeometrySection>> &getBladeSections() const;
};
