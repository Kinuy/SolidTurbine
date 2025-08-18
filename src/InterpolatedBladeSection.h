#pragma once

#include <string>
#include <vector>

#include "AirfoilCoordinate.h"
#include "AirfoilGeometryData.h"

/**
 * @brief Data structure for interpolated blade section with coordinate transformations
 *
 * InterpolatedBladeSection represents a complete blade cross-section that has been
 * processed through geometric interpolation and coordinate transformation. Contains
 * both interpolated geometric parameters and airfoil coordinates in multiple forms
 * for different analysis and visualization purposes.
 *
 * ## Coordinate Types
 * - **Scaled Coordinates**: Airfoil coordinates scaled by chord length
 * - **Transformed Coordinates**: Complete 3D transformation (scaled + rotated + positioned)
 *
 * ## Key Applications
 * - 3D blade surface generation for CFD mesh creation
 * - Blade volume and mass calculations
 * - Aerodynamic analysis with positioned airfoil sections
 * - Visualization and CAD export workflows
 *
 * @see BladeGeometryInterpolator for creation and processing
 * @see AirfoilCoordinate for coordinate data structure
 *
 * @example
 * ```cpp
 * InterpolatedBladeSection section(0.75, 0.12, 25.0, "NACA0012");
 * // Add coordinates through interpolation process
 * section.scaledCoordinates = scaledCoords;
 * section.transformedCoordinates = transformedCoords;
 * ```
 */
struct InterpolatedBladeSection {

    /**
     * @brief Radial position from blade root [m]
     */
    double radius;

    /**
     * @brief Chord length at this radial station [m]
     */
    double chord;

    /**
     * @brief Twist angle at this radial station [deg]
     */
    double twist;

    /**
     * @brief Name/identifier of the airfoil used for this section
     */
    std::string airfoilName;

    /**
     * @brief Airfoil coordinates scaled by chord length
     *
     * 2D coordinates representing the airfoil shape scaled to the
     * appropriate chord length but not yet rotated or positioned.
     */
    std::vector<AirfoilCoordinate> scaledCoordinates;

    /**
     * @brief Fully transformed 3D coordinates
     *
     * 3D coordinates after complete transformation: scaling by chord,
     * rotation by twist angle, and translation to radial position.
     * Ready for 3D analysis and visualization.
     */
    std::vector<AirfoilCoordinate> transformedCoordinates; // After scaling, rotation, and positioning

    /**
     * @brief Constructor initializing basic geometric parameters
     * @param r Radial position [m]
     * @param c Chord length [m]
     * @param t Twist angle [deg]
     * @param name Airfoil identifier
     * @note Coordinate vectors are initialized empty and populated during processing
     */
    InterpolatedBladeSection(double r, double c, double t, const std::string& name)
        : radius(r), chord(c), twist(t), airfoilName(name) {
    }

};
