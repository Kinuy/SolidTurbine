#pragma once


/**
 * @brief Coordinate point structure for airfoil geometry representation
 *
 * AirfoilCoordinate represents a single point in 2D or 3D space for defining
 * airfoil geometry. Supports both traditional 2D airfoil coordinates and
 * extended 3D representations for blade sections and transformed geometries.
 *
 * ## Coordinate System Conventions
 * - **X-axis**: Chordwise direction (typically 0.0-1.0 for normalized coordinates)
 * - **Y-axis**: Thickness direction (positive above, negative below chord line)
 * - **Z-axis**: Spanwise direction (for 3D blade representations)
 *
 * ## 2D vs 3D Usage
 * - **2D Mode**: Traditional airfoil coordinates (z = 0.0)
 * - **3D Mode**: Blade section coordinates after geometric transformations
 *
 * ## Typical Applications
 * - Airfoil coordinate databases and file formats
 * - CFD preprocessing and mesh generation
 * - Blade geometry interpolation and transformation
 * - CAD integration and visualization
 *
 * @see AirfoilGeometryData for coordinate collections
 * @see BladeGeometryInterpolator for 3D transformations
 *
 * @example
 * ```cpp
 * // 2D airfoil coordinate
 * AirfoilCoordinate point2D(0.5, 0.02);  // Mid-chord, upper surface
 *
 * // 3D blade coordinate
 * AirfoilCoordinate point3D(0.5, 0.02, 1.5);  // At 1.5m span
 *
 * // Check dimensionality
 * if (point2D.is2D()) { /* handle 2D case /* }
 *if (point3D.is3D()) { /* handle 3D case /* }
 ```
 */
struct AirfoilCoordinate {

    /**
     * @brief Index of the coordinate in the airfoil geometry
     * Used for referencing specific points in coordinate arrays
	 */
    int index;

    /**
     * @brief X-coordinate (chordwise direction)
     */
    double x;

    /**
     * @brief Y-coordinate (thickness direction)
     */
    double y;

    /**
     * @brief Z-coordinate (spanwise direction for 3D representation)
     */
	double z;

    /**
     * @brief Flag indicating if this coordinate is part of the top surface
     * Used for distinguishing upper and lower surfaces in airfoil geometry
	 */
    bool isTopSurface;

    /**
     * @brief Flag indicating if this coordinate is part of the trailing edge
     * Used for identifying special points in airfoil geometry
	 */
    bool isTrailingEdge;  // or similar flags

    /**
     * @brief Flag indicates if point is last point of TE at top surface -> counting points counter clockwise (TE->Top->LE->Bottom)
     * @return True if point is last point of TE at Top surface
     */
    bool isTETopEdge;

    /**
     * @brief Flag indicates if point is first point of TE at bottom surface -> counting points counter clockwise (TE->Top->LE->Bottom)
     * @return True if point is first point of TE at bottom surface
     */
    bool isTEBottomEdge;



    /**
     * @brief Constructor creating coordinate point with optional Z value
     * @param xVal X-coordinate value
     * @param yVal Y-coordinate value
     * @param zVal Z-coordinate value (default: 0 for 2D coordinates)
     */
    AirfoilCoordinate(int idx, double xVal, double yVal, double zVal=0, bool isTop=NULL, bool isTE=NULL, bool isTETE=NULL, bool isTEBE=NULL) 
        : 
        index(idx), 
        x(xVal), 
        y(yVal), 
        z(zVal), 
        isTopSurface(isTop), 
        isTrailingEdge(isTE),
        isTETopEdge(isTETE),
        isTEBottomEdge(isTEBE)
    {}

    /**
     * @brief Checks if coordinate represents a 2D point
     * @return true if z-coordinate is zero (2D representation)
     */
    bool is2D() const { return z == 0.0; }

    /**
    * @brief Checks if coordinate represents a 3D point
    * @return true if z-coordinate is non-zero (3D representation)
    */
    bool is3D() const { return z != 0.0; }

};