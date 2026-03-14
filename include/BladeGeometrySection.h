#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <numbers>
#include "AirfoilCoordinate.h"
#include "AirfoilPolarData.h"
#include "AirfoilGeometryData.h"
// #include "Angles.h"

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
struct BladeGeometrySection
{

    /**
     * @brief Section type identifier (always "DEF" for data rows)
     */
    std::string type; // "DEF"

    /**
     * @brief Radial position from blade root [m]
     */
    double bladeRadius; // [m]

    /**
     * @brief Chord length at this radial station [m]
     */
    double chord; // [m]

    /**
     * @brief Twist angle relative to reference [rad], read in as degrees and converted to radians
     */
    // double twist;               // [rad]
    double twist; // [rad]

    /**
     * @brief Relative thickness as percentage of chord [%]
     */
    double relativeThickness; // [%]

    /**
     * @brief X-coordinate of t/4 point [m]
     */
    double xt4; // [m]

    /**
     * @brief Y-coordinate of t/4 point [m]
     */
    double yt4; // [m]

    /**
     * @brief X-coordinate of pitch change bearing axis [m]
     */
    double pcbaX; // [m]

    /**
     * @brief Y-coordinate of pitch change bearing axis [m]
     */
    double pcbaY; // [m]

    /**
     * @brief Relative position of twist axis along chord [%]
     * @note Zero-initialised when loading from BladeGeometry_E format
     */
    double relativeTwistAxis = 0.0; // [%]

    // -------------------------------------------------------------------------
    // Extended fields present in BladeGeometry_E format only.
    // All fields are zero-initialised; standard-format loads leave them at 0.
    // -------------------------------------------------------------------------

    /** @brief Absolute thickness [mm] (E-format col 4) */
    double thicknessAbs_mm   = 0.0;
    /** @brief Trailing-edge thickness [mm] (E-format col 8) */
    double trailingEdge_mm   = 0.0;
    /** @brief Pre-bend offset [mm] (E-format col 9) */
    double prebend_mm        = 0.0;
    /** @brief Chord including trailing-edge strip [m] (E-format col 10) */
    double chordWithTES_m    = 0.0;
    /** @brief Pre-bend angle gamma [deg] (E-format col 11) */
    double gammaPrebend_deg  = 0.0;
    /** @brief Pre-bend radius [m] (E-format col 12) */
    double radiusPrebend_m   = 0.0;
    /** @brief Relative thickness at 0.01 chord (E-format col 13) */
    double relThick001       = 0.0;
    /** @brief Relative thickness at 0.1 chord (E-format col 14) */
    double relThick01        = 0.0;
    /** @brief Trailing-edge angle [deg] (E-format col 15) */
    double teAngle_deg       = 0.0;
    /** @brief Add-ons flag / value (E-format col 16) */
    std::string addOns            = "";

    /**
     * @brief Name/identifier of the airfoil used for this section
     *
     * Used to reference the specific airfoil geometry associated with this
     * blade section, typically defined in an external airfoil database.
     */
    std::string airfoilName; // Airfoil identifier (e.g., "NACA0012", "S809")

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
     * @brief Constructor from tokenized file line — standard format (10+ tokens)
     *
     * Expects tokens produced by BladeGeometryParser for the standard format:
     * [0]=DEF [1]=bladeRadius [2]=chord [3]=twist° [4]=relThick% [5]=xt4
     * [6]=yt4 [7]=pcbaX [8]=pcbaY [9]=relativeTwistAxis [10?]=airfoilName
     *
     * E-format columns beyond index 9 are left at their zero-initialised defaults.
     *
     * @param tokens Vector of string tokens from parsed file line
     * @throws std::invalid_argument if fewer than 10 tokens provided
     * @throws std::invalid_argument if numeric conversion fails
     */
    BladeGeometrySection(const std::vector<std::string> &tokens)
    {
        if (tokens.size() < 10)
        {
            throw std::invalid_argument("Insufficient columns for blade geometry row");
        }

        type              = tokens[0];
        bladeRadius       = std::stod(tokens[1]);
        chord             = std::stod(tokens[2]);
        twist             = std::stod(tokens[3]) * std::numbers::pi / 180.0;
        relativeThickness = std::stod(tokens[4]);
        xt4               = std::stod(tokens[5]);
        yt4               = std::stod(tokens[6]);
        pcbaX             = std::stod(tokens[7]);
        pcbaY             = std::stod(tokens[8]);
        relativeTwistAxis = std::stod(tokens[9]);

        if (tokens.size() > 10)
            airfoilName = tokens[10];
        else
            airfoilName = "R_" + std::to_string(bladeRadius) + "_m_RelThick_" + std::to_string(relativeThickness);
    }

    /**
     * @brief Copy constructor
     * @param other The BladeGeometrySection to copy from
     */
    BladeGeometrySection(const BladeGeometrySection &other)
        : type(other.type), bladeRadius(other.bladeRadius), chord(other.chord),
          twist(other.twist), relativeThickness(other.relativeThickness),
          xt4(other.xt4), yt4(other.yt4), pcbaX(other.pcbaX), pcbaY(other.pcbaY),
          relativeTwistAxis(other.relativeTwistAxis),
          thicknessAbs_mm(other.thicknessAbs_mm), trailingEdge_mm(other.trailingEdge_mm),
          prebend_mm(other.prebend_mm), chordWithTES_m(other.chordWithTES_m),
          gammaPrebend_deg(other.gammaPrebend_deg), radiusPrebend_m(other.radiusPrebend_m),
          relThick001(other.relThick001), relThick01(other.relThick01),
          teAngle_deg(other.teAngle_deg), addOns(other.addOns),
          airfoilName(other.airfoilName), coordinates(other.coordinates),
          scaledCoordinates(other.scaledCoordinates),
          transformedCoordinates(other.transformedCoordinates)
    {
    }
};