#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <variant>
#include "AirfoilGeometryData.h"


/**
 * @brief Factory class for airfoil geometry interpolation
 */
class AirfoilGeometryInterpolationFactory {

private:

    /**
	 * @brief Finds the pair of airfoil geometries to interpolate between based on target thickness
	 * @return Pair of pointers to the left and right airfoil geometries for interpolation
     */
    static std::pair<const AirfoilGeometryData*, const AirfoilGeometryData*> findInterpolationPair(
        const std::vector<std::unique_ptr<AirfoilGeometryData>>& airfoilGeometries,
        double targetThickness);

public:

    /**
	 * @brief Create interpolated airfoil geometry between two existing geometries
	 * @return Interpolated airfoil geometry data between two existing geometries
     */
    static std::unique_ptr<AirfoilGeometryData> createInterpolatedAirfoilGeometry(
        const AirfoilGeometryData& leftGeometry,
        const AirfoilGeometryData& rightGeometry,
        double targetThickness);

    /**
	 * @brief Find best matching airfoil geometry from collection
	 * @return Pointer to the best matching airfoil geometry data
     */
    static const AirfoilGeometryData* findBestMatch(
        const std::vector<std::unique_ptr<AirfoilGeometryData>>& geometries,
        double targetThickness);

    /**
	 * @brief Get airfoil geometry for section (exact match or interpolated)
	 * @return Variant containing either a pointer to the best matching airfoil geometry or a new interpolated geometry
     */
    static std::unique_ptr<AirfoilGeometryData> getAirfoilGeometryForSection(
        const std::vector<std::unique_ptr<AirfoilGeometryData>>& geometries,
        double targetThickness,
        double tolerance = 0.001);

};

