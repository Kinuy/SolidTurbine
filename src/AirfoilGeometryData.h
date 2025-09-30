#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <numbers>
#include "IStructuredData.h"
#include "AirfoilCoordinate.h"
#include "AirfoilMarker.h"
#include "AirfoilGeometryFileInfo.h"

#include "InterpolationMethod.h"
#include "AkimaSplineInterpolationStrategy.h"
#include "CubicSplineInterpolationStrategy.h"
#include "LinearInterpolationStrategy.h"
#include "MonotonicCubicSplineInterpolationStrategy.h"

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
     * @brief Complete set of scaled (with chord in m and max thickness in m) coordinate points defining the geometry
     */
	std::vector<AirfoilCoordinate> scaledCoordinates;

    /**
     * @brief Header lines from source file (comments and metadata)
     */
    std::vector<std::string> headers;

    /**
     * @brief Orientation of airfoil coordinates -> default is from TE->top->LE->bottom->TE
     */
    bool coordinateOrientation;

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
     * @brief Sets z coordinate for all coordinates in vector. This is needed for section interpolation.
     */
    void setZCoordinates(double z);

    /**
     * @brief Sets the index of a given marker.
     */
    void setMarkerIndex(std::string type, int idx);

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
    void addCoordinate(int idx, double x, double y, double z, bool isTop, bool isTE, bool isTETE, bool isTEBE);

    /**
     * @brief Add airfoil nose point (0,0) to coordinates and insert copy of last top surface point to beginning of bottom surface point list  
     */
    void addAirfoilNosePoint(const bool nosePointExists);

    /**
     * @brief Add airfoil nose point to coordinates
     */
    void insertNosePointAtOrigin(const bool nosePointExists);

    /**
     * @brief Add airfoil nose point interpolated y value to coordinates
     */
    void insertNosePointWithInterpolation(const bool nosePointExists);

    /**
     * @brief Move all aifoil coordinates by x nd y value of nose so that nose is centered in (0,0) position.
     */
    void moveAllCoordinatesByNoseXY(const bool centeredNosePointExists);

    /**
     * @brief Check orientation of coordinates start from TE top to Nose to bottom to top or reversed, if not counterclockwise reverse!
     */
    void orientationToDefaultCounterClockwiseOrientation();

    /**
     * @brief Find and assign LE
     */
    void findAndAssignLE();

    /**
     * @brief Find and assign top or bottom flag to airfoil coordinate
     */
    void findAndAssignTopBottom();

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
     * @brief Gets all acled and rotated coordinate points
     * @return Const reference to vector of AirfoilCoordinate objects
     */
	const std::vector<AirfoilCoordinate>& getScaledAndRotatedCoordinates() const;

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

    /**
    * @brief Check if airfoil coordinate is part of top surface
    * @return true if coordinate is part of top surface, false if it is part of bootm surface
    */
    bool coordinateIsTop(const int idx, const int topSide) const;

    /**
    * @brief Check if airfoil coordinate is part of trailing edge
    * @return true if coordinate is part , false if it is not part of trailing edge
    */
    bool coordinateIsTE(const double x) const;

    /**
    * @brief Finds and assignes trailing edge top edge (TETE) and trailing edge bottom edge (TEBE) point
    */
    void findAndAssignTETEAndTEBEPoints();

    /**
	 * @brief Applies a twist angle to the airfoil geometry around the quarter-chord point and pitch axis position 
	 *  
	*/
    void applyTwistAroundQuarterChord(double twistAngleDegrees, double pitchAxis);

    /**
     * @brief Applies a twist angle to the airfoil geometry around the quarter-chord point
	 * @param twistAngleDegrees Twist angle in degrees (positive = leading edge up), negative = leading edge down, 0 = no twist, pivotX and pivotY set rotation point
     * @note Modifies all coordinates in place
	 */
    void applyTwistAngleAroundPivotPoint(double twistAngleDegrees, double pivotX, double pivotY);

    /**
	 * @brief Applies Scaling of coordiante points x-> with chord in m and y with abs thickness in m of airfoil, if exists z with radius in m
	 * @param chordLength Chord length in m
	 * @param maxThickness Maximum thickness in m
	 * @param targetRadius Target radius in m (default = 0 for 2D airfoils)
     * @note Modifies all coordinates in place
     */
	void applyScalingWithChordAndMaxThickness(double chordLength, double maxThickness, double targetRadius = 0);

    /*
    * @brief Apply translation in x and y direction to meet prebend and sweep position of airfoil
	* @param pcbaX Prebend value in m (translation in x direction), positive towards top of blade, negative towards bottom of blade
	* @param pcbaY Sweep value in m (translation in y direction), positive towards leading edge, negative towards trailing edge
    * @note Modifies all coordinates in place
    */
    void applyTranslationXY(double pcbaX, double pcbaY);

    /**
     * @brief Main simplified interpolation method
     * @param leftGeometry Left airfoil geometry data for interpolation
     * @param rightGeometry Right airfoil geometry data for interpolation
     * @param targetThickness Target relative thickness for the new Geometry
     * @return Interpolated geometry data between two existing airfoil geometries
     */
    static std::unique_ptr<AirfoilGeometryData> interpolateBetweenGeometries(
        const AirfoilGeometryData& leftGeometry, const AirfoilGeometryData& rightGeometry, double targetThickness);

    /**
     * @brief Separates coordinates into upper and lower surfaces
     * @param nodes Vector of AirfoilCoordinate objects to separate
     * @return Pair of vectors containing upper and lower surface coordinates
	 */
    static std::pair<std::vector<AirfoilCoordinate>, std::vector<AirfoilCoordinate>> separateTopBottom(const std::vector<AirfoilCoordinate>& nodes);

    /**
     * @brief Helper function to get unique x-coordinates from two surfaces
     * @param surface1 surface top
	 * @param surface2 surface bottom
	 * @return Vector of x-coordinates that are unique across both surfaces
	 */
    static std::vector<double> getUniqueXCoordinates(const std::vector<AirfoilCoordinate>& surface1, const std::vector<AirfoilCoordinate>& surface2);

    /**
     * @brief Interpolates coordinates between two airfoil surfaces
     * @param surface1 First surface coordinates (e.g., top surface)
     * @param surface2 Second surface coordinates (e.g., bottom surface)
     * @param xCoords Vector of unique x-coordinates for interpolation
     * @param percent Interpolation factor (0.0 = surface1, 1.0 = surface2)
     * @param isTopSurface Flag indicating if the interpolation is for the top surface
     * @return Vector of interpolated AirfoilCoordinate objects
	 */
    static std::vector<AirfoilCoordinate> interpolateSurface(const std::vector<AirfoilCoordinate>& surface1, const std::vector<AirfoilCoordinate>& surface2, const std::vector<double>& xCoords, double percent, bool isTopSurface);

    /**
     * @brief Combines top and bottom surfaces into a single vector
     * @param top Vector of AirfoilCoordinate objects for the top surface
     * @param bottom Vector of AirfoilCoordinate objects for the bottom surface
     * @return Combined vector of AirfoilCoordinate objects representing the full airfoil geometry
	 */
    static std::vector<AirfoilCoordinate> combineTopBottomSurfaces( std::vector<AirfoilCoordinate>& top,  std::vector<AirfoilCoordinate>& bottom);

};

