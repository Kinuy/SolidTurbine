#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <variant>
#include "AirfoilPolarData.h"


/**
 * @brief Factory class for airfoil polar interpolation
 */
class AirfoilPolarInterpolationFactory {

private:

    /**
	 * @brief Finds the pair of polars to interpolate between based on target thickness
	 * @param polars Collection of available polar data
	 * @param targetThickness Target relative thickness for the section
	 * @return Pair of pointers to the left and right polar data for interpolation
     */
    static std::pair<const AirfoilPolarData*, const AirfoilPolarData*> findInterpolationPair(
            const std::vector<std::unique_ptr<AirfoilPolarData>>& polars,
            double targetThickness);

public:

    /**
     * @brief Create interpolated polar data between two existing polars
	 * @param leftPolar Left polar data for interpolation
	 * @param rightPolar Right polar data for interpolation
	 * @param targetThickness Target relative thickness for the new polar
	 * @return Interpolated polar data between two existing polars
	 */
    static std::unique_ptr<AirfoilPolarData> createInterpolatedPolar(
        const AirfoilPolarData& leftPolar,
        const AirfoilPolarData& rightPolar,
        double targetThickness);

    /**
     * @brief Find best matching polar from collection
	 * @param polars Collection of available polar data
	 * @param targetThickness Target relative thickness for the section
	 * @return Pointer to the best matching polar data
     */
    static const AirfoilPolarData* findBestMatch(
        const std::vector<std::unique_ptr<AirfoilPolarData>>& polars,
        double targetThickness);

    /**
     * @brief Get polar for section (exact match or interpolated)
	 * @param polars Collection of available polar data
	 * @param targetThickness Target relative thickness for the section
	 * @param tolerance Tolerance for matching thickness (default 0.001)
	 * @return Variant containing either a pointer to the best matching polar or a new interpolated polar
     */
    static std::unique_ptr<AirfoilPolarData> getPolarForSection(
            const std::vector<std::unique_ptr<AirfoilPolarData>>& polars,
            double targetThickness,
            double tolerance = 0.001);

};
