#pragma once

#include <string>
#include <vector>

#include "AirfoilCoordinate.h"
#include "AirfoilGeometryData.h"

// Represents a single interpolated blade section with its properties and transformed airfoil coordinates
struct InterpolatedBladeSection {
    double radius;
    double chord;
    double twist;
    std::string airfoilName;
    std::vector<AirfoilCoordinate> scaledCoordinates;
    std::vector<AirfoilCoordinate> transformedCoordinates; // After scaling, rotation, and positioning

    InterpolatedBladeSection(double r, double c, double t, const std::string& name)
        : radius(r), chord(c), twist(t), airfoilName(name) {
    }
};
