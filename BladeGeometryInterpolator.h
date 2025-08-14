#pragma once

#define _USE_MATH_DEFINES

#include <vector>
#include <memory>
#include <math.h>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "BladeGeometryData.h"
#include "AirfoilGeometryData.h"
#include "AirfoilCoordinate.h"
#include "BladeGeometrySection.h"
#include "IInterpolationStrategy.h"
#include "InterpolationMethod.h"
#include "InterpolatedBladeSection.h"
#include "LinearInterpolationStrategy.h"
#include "CubicSplineInterpolationStrategy.h"
#include "AkimaSplineInterpolationStrategy.h"
#include "MonotonicCubicSplineInterpolationStrategy.h"



class BladeGeometryInterpolator {

private:

    const BladeGeometryData& bladeGeometry;

    std::vector<const AirfoilGeometryData*>& airfoilGeometries;

    std::unique_ptr<IInterpolationStrategy> interpolationStrategy;

    // Transform coordinates: scale by chord, rotate by twist, translate to radial position
    AirfoilCoordinate transformCoordinate(const AirfoilCoordinate& coord,
        double chord, double twist, double radius) const;

    // Find the most appropriate airfoil for a given blade section
    const AirfoilGeometryData* selectAirfoilForSection(const BladeGeometrySection& section) const;

    // Create interpolation strategy based on method
    std::unique_ptr<IInterpolationStrategy> createInterpolationStrategy(InterpolationMethod method) const;

public:

    BladeGeometryInterpolator(const BladeGeometryData& bladeGeom,
        std::vector<const AirfoilGeometryData*>& airfoilGeoms,
        InterpolationMethod method = InterpolationMethod::LINEAR);

    // Change interpolation method dynamically
    void setInterpolationMethod(InterpolationMethod method);

    std::string getCurrentInterpolationMethod() const;

    // Generate interpolated blade sections for all geometry points
    std::vector<InterpolatedBladeSection> interpolateAllSections() const;

    // Interpolate a single blade section
    InterpolatedBladeSection interpolateSection(const BladeGeometrySection& section) const;

    // Generate blade surface points for 3D visualization/meshing
    std::vector<std::vector<AirfoilCoordinate>> generateBladeSurface() const;

    // Calculate blade volume (simplified)
    double calculateBladeVolume() const;

    // Generate coordinates for specific span positions using selected interpolation method
    InterpolatedBladeSection interpolateAtRadius(double targetRadius) const;

    // Generate dense blade sections using interpolation
    std::vector<InterpolatedBladeSection> generateDenseBladeSections(size_t numSections) const;

    // Compare different interpolation methods
    void compareInterpolationMethods(double targetRadius) const;

    // Export blade geometry for CAD/CFD (simplified format)
    void exportBladeGeometry(const std::string& filename) const;

};

