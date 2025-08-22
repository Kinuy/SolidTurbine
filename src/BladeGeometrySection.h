#pragma once

#include<string>
#include<vector>
#include<stdexcept>
#include "AirfoilCoordinate.h"
#include "AirfoilPolarData.h"
#include "AirfoilGeometryData.h"

/**
 * @brief Data structure representing a single radial section of blade geometry
 *
 * BladeGeometrySection contains all geometric parameters defining a blade
 * cross-section at a specific radial position. Used in turbomachinery design
 * and analysis for defining blade shape along the span.
 *
 * ## Parameter Categories
 * - **Position**: bladeRadius defines spanwise location
 * - **Basic Geometry**: chord, twist, thickness define section shape
 * - **Airfoil Details**: xt4, yt4 specify airfoil-specific coordinates
 * - **Reference Points**: pcbaX, pcbaY define pitch change bearing axis
 * - **Axes**: relativeTwistAxis defines twist reference location
 *
 * @note All parameters are parsed from tokenized file lines with validation
 * @note Constructor requires exactly 10 tokens for complete section definition
 *
 * @example
 * ```cpp
 * std::vector<std::string> tokens = {"DEF", "0.5", "0.12", "25.0", "12.5",
 *                                   "0.06", "0.015", "0.04", "0.008", "50.0"};
 * BladeGeometrySection section(tokens);
 * ```
 */
struct BladeGeometrySection {

    /**
     * @brief Section type identifier (always "DEF" for data rows)
     */
    std::string type;           // "DEF"

    /**
     * @brief Radial position from blade root [m]
     */
    double bladeRadius;         // [m]

    /**
     * @brief Chord length at this radial station [m]
     */
    double chord;               // [m] 

    /**
     * @brief Twist angle relative to reference [deg]
     */
    double twist;               // [deg]

    /**
     * @brief Relative thickness as percentage of chord [%]
     */
    double relativeThickness;   // [%]

    /**
     * @brief X-coordinate of t/4 point [m]
     */
    double xt4;                 // [m]

    /**
     * @brief Y-coordinate of t/4 point [m]
     */
    double yt4;                 // [m]

    /**
     * @brief X-coordinate of pitch change bearing axis [m]
     */
    double pcbaX;               // [m]

    /**
     * @brief Y-coordinate of pitch change bearing axis [m]
     */
    double pcbaY;               // [m]

    /**
     * @brief Relative position of twist axis along chord [%]
     */
    double relativeTwistAxis;   // [%]

    /**
     * @brief Name/identifier of the airfoil used for this section
     *
     * Used to reference the specific airfoil geometry associated with this
     * blade section, typically defined in an external airfoil database.
	 */
	std::string airfoilName;    // Airfoil identifier (e.g., "NACA0012", "S809")

    /**
     * @brief Relative airfoil coordinates for this section
     */
    std::vector<AirfoilCoordinate> coordinates;

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
     * @brief Airfoil performance data points for this section
     */
    std::unique_ptr<AirfoilPolarData> airfoilPolar;

    /**
     * @brief Airfoil geometry data for this section
     *
     * Contains complete airfoil geometry including coordinates, markers,
     * and metadata. Used for geometric analysis and visualization.
	 */
	std::unique_ptr<AirfoilGeometryData> airfoilGeometry;

    /**
     * @brief Default constructor
     */
    BladeGeometrySection() = default;

    /**
     * @brief Constructor from tokenized file line
     * @param tokens Vector of string tokens from parsed file line
     * @throws std::invalid_argument if fewer than 10 tokens provided
     * @throws std::invalid_argument if numeric conversion fails
     */
    BladeGeometrySection(const std::vector<std::string>& tokens) {
        if (tokens.size() < 10) {
            throw std::invalid_argument("Insufficient columns for blade geometry row");
        }

        type = tokens[0];
        bladeRadius = std::stod(tokens[1]);
        chord = std::stod(tokens[2]);
        twist = std::stod(tokens[3]);
        relativeThickness = std::stod(tokens[4]);
        xt4 = std::stod(tokens[5]);
        yt4 = std::stod(tokens[6]);
        pcbaX = std::stod(tokens[7]);
        pcbaY = std::stod(tokens[8]);
        relativeTwistAxis = std::stod(tokens[9]);
    }

    /**
     * @brief Copy constructor
     * @param other The BladeGeometrySection to copy from
     */
    BladeGeometrySection(const BladeGeometrySection& other)
        : type(other.type)
        , bladeRadius(other.bladeRadius)
        , chord(other.chord)
        , twist(other.twist)
        , relativeThickness(other.relativeThickness)
        , xt4(other.xt4)
        , yt4(other.yt4)
        , pcbaX(other.pcbaX)
        , pcbaY(other.pcbaY)
        , relativeTwistAxis(other.relativeTwistAxis)
        , airfoilName(other.airfoilName)
        , coordinates(other.coordinates)
        , scaledCoordinates(other.scaledCoordinates)
        , transformedCoordinates(other.transformedCoordinates)
    {
    }

};