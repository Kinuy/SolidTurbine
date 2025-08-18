#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "IStructuredData.h"
#include "AirfoilCoordinate.h"
#include "AirfoilMarker.h"
#include "AirfoilGeometryFileInfo.h"

/**
 * @brief Structured data container for airfoil geometry coordinates and metadata
 *
 * AirfoilGeometryData implements IStructuredData to store and manage complete
 * airfoil coordinate datasets including geometric points, special markers, and
 * metadata. Provides access to airfoil surface definitions and geometric
 * analysis capabilities for aerodynamic applications.
 *
 * ## Key Features
 * - **Coordinate Storage**: Complete airfoil coordinate point collections
 * - **Geometry Markers**: Special point identification (leading edge, trailing edge, etc.)
 * - **Surface Extraction**: Methods to separate upper and lower surface coordinates
 * - **Geometric Analysis**: Chord length, thickness, and dimensional calculations
 *
 * ## Coordinate System
 * Typically uses normalized coordinates where:
 * - X-axis: Chordwise direction (0.0 at leading edge, 1.0 at trailing edge)
 * - Y-axis: Thickness direction (positive above, negative below chord line)
 *
 * ## Use Cases
 * - CFD preprocessing and mesh generation
 * - Blade element momentum (BEM) analysis
 * - Airfoil performance database management
 * - Geometric property calculation and validation
 * - CAD integration and visualization
 *
 * @see IStructuredData for the base interface
 * @see AirfoilCoordinate for individual coordinate point structure
 * @see AirfoilMarker for geometry marker definitions
 * @see AirfoilGeometryParser for file parsing
 *
 * @example
 * ```cpp
 * AirfoilGeometryData airfoil;
 * airfoil.setName("NACA0012");
 * airfoil.addCoordinate(1.0, 0.0);  // Trailing edge
 * airfoil.addMarker("TE", 0);       // Mark trailing edge
 *
 * auto upperSurface = airfoil.getUpperSurface();
 * double chord = airfoil.getChordLength();
 * ```
 */
class AirfoilGeometryData : public IStructuredData {

private:

    /**
     * @brief Airfoil name or identifier (e.g., "NACA0012", "S809")
     */
    std::string name;

    /**
     * @brief Relative thickness as percentage of chord length
     */
    double relativeThickness;

    /**
     * @brief Collection of geometry markers identifying special points
     */
    std::vector<AirfoilMarker> markers;

    /**
     * @brief Complete set of airfoil coordinate points defining the geometry
     */
    std::vector<AirfoilCoordinate> coordinates;

    /**
     * @brief Header lines from source file (comments and metadata)
     */
    std::vector<std::string> headers;

    /**
     * @brief Finds geometry marker by type identifier
     * @param type Marker type to search for (e.g., "LE", "TE", "TESSMAX")
     * @return AirfoilMarker object of specified type
     * @throws std::runtime_error if marker type not found
     */
    AirfoilMarker getMarkerByType(const std::string& type) const;


public:

    /**
     * @brief Sets the airfoil name/identifier
     * @param n Airfoil name string
     */
    void setName(const std::string& name);

    /**
     * @brief Sets the relative thickness percentage
     * @param thickness Relative thickness value
     */
    void setRelativeThickness(double thickness);

    /**
     * @brief Adds a header line to the dataset
     * @param header Header string to add
     */
    void addHeader(const std::string& header);

    /**
     * @brief Adds a geometry marker to the dataset
     * @param type Marker type identifier (e.g., "LE", "TE", "TESSMAX")
     * @param index Index position in coordinate array
     */
    void addMarker(const std::string& type, int index);

    /**
     * @brief Adds a coordinate point to the airfoil geometry
     * @param x X-coordinate value
     * @param y Y-coordinate value
     */
    void addCoordinate(double x, double y);

    /**
     * @brief Gets the airfoil name/identifier
     * @return Airfoil name string
     */
    const std::string& getName() const;

    /**
     * @brief Gets the relative thickness percentage
     * @return Relative thickness value
     */
    double getRelativeThickness() const;

    /**
     * @brief Gets all geometry markers
     * @return Const reference to vector of AirfoilMarker objects
     */
    const std::vector<AirfoilMarker>& getMarkers() const;

    /**
     * @brief Gets all coordinate points
     * @return Const reference to vector of AirfoilCoordinate objects
     */
    const std::vector<AirfoilCoordinate>& getCoordinates() const;

    /**
     * @brief Gets all header lines
     * @return Const reference to vector of header strings
     */
    const std::vector<std::string>& getHeaders() const;

    /**
     * @brief Gets the data type identifier
     * @return Always returns "AirfoilGeometry"
     */
    std::string getTypeName() const override;

    /**
     * @brief Gets the number of coordinate points
     * @return Number of coordinates in the dataset
     */
    size_t getRowCount() const override;

    /**
     * @brief Gets leading edge coordinate using LE marker
     * @return AirfoilCoordinate at leading edge position
     * @throws std::runtime_error if LE marker not found or index invalid
     */
    AirfoilCoordinate getLeadingEdge() const;

    /**
     * @brief Gets trailing edge coordinate using TE marker
     * @return AirfoilCoordinate at trailing edge position
     * @throws std::runtime_error if TE marker not found or index invalid
     */
    AirfoilCoordinate getTrailingEdge() const;

    /**
     * @brief Extracts upper surface coordinates (positive Y values)
     * @return Vector of coordinates representing upper airfoil surface
     * @note Simplified implementation using Y-coordinate sign
     */
    std::vector<AirfoilCoordinate> getUpperSurface() const;

    /**
     * @brief Extracts lower surface coordinates (negative Y values)
     * @return Vector of coordinates representing lower airfoil surface
     * @note Simplified implementation using Y-coordinate sign
     */
    std::vector<AirfoilCoordinate> getLowerSurface() const;

    /**
     * @brief Calculates chord length from leading to trailing edge
     * @return Chord length as distance between LE and TE markers
     * @throws std::runtime_error if LE or TE markers not found
     */
    double getChordLength() const;

    /**
     * @brief Calculates maximum thickness of the airfoil
     * @return Maximum thickness value found between upper and lower surfaces
     * @note Simplified calculation comparing surfaces at matching X positions
     */
    double getMaxThickness() const;

};

