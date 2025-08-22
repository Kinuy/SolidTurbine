#include "AirfoilPolarInterpolationFactory.h"


std::pair<const AirfoilPolarData*, const AirfoilPolarData*> AirfoilPolarInterpolationFactory::findInterpolationPair(
    const std::vector<std::unique_ptr<AirfoilPolarData>>& polars,
    double targetThickness) {

    if (polars.size() < 2) {
        throw std::runtime_error("Need at least 2 polars for interpolation");
    }

    // Sort polars by thickness
    std::vector<const AirfoilPolarData*> sortedPolars;
    for (const auto& polar : polars) {
        sortedPolars.push_back(polar.get());
    }

    std::sort(sortedPolars.begin(), sortedPolars.end(),
        [](const AirfoilPolarData* a, const AirfoilPolarData* b) {
            return a->getRelativeThickness() < b->getRelativeThickness();
        });

    // Find surrounding polars
    for (size_t i = 0; i < sortedPolars.size() - 1; ++i) {
        double lowerThickness = sortedPolars[i]->getRelativeThickness();
        double upperThickness = sortedPolars[i + 1]->getRelativeThickness();

        if (targetThickness >= lowerThickness && targetThickness <= upperThickness) {
            return { sortedPolars[i], sortedPolars[i + 1] };
        }
    }

    // If outside range, use closest boundary pair
    if (targetThickness < sortedPolars.front()->getRelativeThickness()) {
        return { sortedPolars[0], sortedPolars[1] };
    }
    else {
        size_t last = sortedPolars.size() - 1;
        return { sortedPolars[last - 1], sortedPolars[last] };
    }
}


std::unique_ptr<AirfoilPolarData> AirfoilPolarInterpolationFactory::createInterpolatedPolar(
    const AirfoilPolarData& leftPolar,
    const AirfoilPolarData& rightPolar,
    double targetThickness) {

    if (leftPolar.getPolarData().empty() || rightPolar.getPolarData().empty()) {
        throw std::runtime_error("Cannot interpolate between empty polar data");
    }

    return AirfoilPolarData::interpolateBetweenPolars(
        leftPolar, rightPolar, targetThickness);
}


const AirfoilPolarData* AirfoilPolarInterpolationFactory::findBestMatch(
    const std::vector<std::unique_ptr<AirfoilPolarData>>& polars,
    double targetThickness) {

    if (polars.empty()) {
        throw std::runtime_error("No polar data available");
    }

    const AirfoilPolarData* bestMatch = polars[0].get();
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


std::unique_ptr<AirfoilPolarData> AirfoilPolarInterpolationFactory::getPolarForSection(
    const std::vector<std::unique_ptr<AirfoilPolarData>>& polars,
    double targetThickness,
    double tolerance) {

    // Try exact match first
    //const auto* exactMatch = findBestMatch(polars, targetThickness);
    //if (std::abs(exactMatch->getRelativeThickness() - targetThickness) <= tolerance) {
    //    return exactMatch;
    //}

    // Find interpolation pair
    auto [leftPolar, rightPolar] = findInterpolationPair(polars, targetThickness);

    return createInterpolatedPolar(*leftPolar, *rightPolar, targetThickness);
}