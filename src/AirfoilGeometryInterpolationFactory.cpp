#include "AirfoilGeometryInterpolationFactory.h"


std::pair<const AirfoilGeometryData*, const AirfoilGeometryData*>
AirfoilGeometryInterpolationFactory::findInterpolationPair(const std::vector<const AirfoilGeometryData*>& airfoilGeometries, double targetThickness) {
    if (airfoilGeometries.size() < 2) {
        throw std::runtime_error("Need at least 2 airfoils for interpolation");
    }

    std::vector<const AirfoilGeometryData*> sortedAirfoils;
    for (const auto& airfoil : airfoilGeometries) {
        sortedAirfoils.push_back(airfoil);
    }

    // Sort airfoils by thickness for easier processing
    std::sort(sortedAirfoils.begin(), sortedAirfoils.end(),
        [](const AirfoilGeometryData* a, const AirfoilGeometryData* b) {
            return a->getRelativeThickness() < b->getRelativeThickness();
        });

    // Find surrounding airfoils
    for (size_t i = 0; i < sortedAirfoils.size() - 1; ++i) {
        double thicknessLower = sortedAirfoils[i]->getRelativeThickness();
        double thicknessUpper = sortedAirfoils[i + 1]->getRelativeThickness();

        if (targetThickness >= thicknessLower && targetThickness <= thicknessUpper) {
            return { sortedAirfoils[i], sortedAirfoils[i + 1] };
        }
    }

    // If outside range, use closest boundary pair
    if (targetThickness < sortedAirfoils.front()->getRelativeThickness()) {
        return { sortedAirfoils[0], sortedAirfoils[1] };
    }
    else {
        size_t last = sortedAirfoils.size() - 1;
        return { sortedAirfoils[last - 1], sortedAirfoils[last] };
    }
}


std::unique_ptr<AirfoilGeometryData> AirfoilGeometryInterpolationFactory::createInterpolatedAirfoilGeometry(
    const AirfoilGeometryData& leftGeometry,
    const AirfoilGeometryData& rightGeometry,
    double targetThickness) {

    if (leftGeometry.getCoordinates().empty() || rightGeometry.getCoordinates().empty()) {
        throw std::runtime_error("Cannot interpolate between empty polar data");
    }
	// This is the static core method call for the interpolation logic
    return AirfoilGeometryData::interpolateBetweenGeometries(
        leftGeometry, rightGeometry, targetThickness);
}


const AirfoilGeometryData* AirfoilGeometryInterpolationFactory::findBestMatch(
    const std::vector<std::unique_ptr<AirfoilGeometryData>>& polars,
    double targetThickness) {

    if (polars.empty()) {
        throw std::runtime_error("No polar data available");
    }

    const AirfoilGeometryData* bestMatch = polars[0].get();
    double minDifference = std::abs(bestMatch->getRelativeThickness() - targetThickness);

    for (const auto& polar : polars) {
        double difference = std::abs(polar->getRelativeThickness() - targetThickness);
        if (difference < minDifference) {
            minDifference = difference;
            bestMatch = polar.get();
        }
    }

    return bestMatch;
}


std::unique_ptr<AirfoilGeometryData> AirfoilGeometryInterpolationFactory::getAirfoilGeometryForSection(
    const std::vector<const AirfoilGeometryData*>& airfoilGeometries,
    double targetThickness,
    double tolerance) {

    // Try exact match first
    //const auto* exactMatch = findBestMatch(airfoilGeometries, targetThickness);
    //if (std::abs(exactMatch->getRelativeThickness() - targetThickness) <= tolerance) {
    //    return exactMatch;
    //}

    // Find interpolation pair
    auto [leftGeometry, rightGeometry] = findInterpolationPair(airfoilGeometries, targetThickness);

    return createInterpolatedAirfoilGeometry(*leftGeometry, *rightGeometry, targetThickness);
}