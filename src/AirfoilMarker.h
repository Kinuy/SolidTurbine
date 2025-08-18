#pragma once

#include <string>

/**
 * @brief Marker structure for identifying special points on airfoil geometry
 *
 * AirfoilMarker provides a way to tag specific locations in airfoil coordinate
 * arrays with semantic meaning. Used for marking geometric features, tessellation
 * points, and other important locations during airfoil processing and analysis.
 *
 * ## Common Marker Types
 * - **TEE**: Trailing edge point
 * - **TESSMAX**: Maximum tessellation density point
 * - **TESS**: General tessellation control points
 * - **LE**: Leading edge point
 * - **SPLIT**: Pressure/suction surface split points
 *
 * ## Usage Context
 * Typically used in airfoil geometry processing pipelines where specific
 * coordinate points need special treatment for meshing, analysis, or
 * geometric operations.
 *
 * @see AirfoilGeometryData for coordinate array context
 * @see AirfoilCoordinate for individual point data
 *
 * @example
 * ```cpp
 * AirfoilMarker trailingEdge("TEE", 127);  // Mark trailing edge at index 127
 * AirfoilMarker leadingEdge("LE", 64);     // Mark leading edge at index 64
 * ```
 */
struct AirfoilMarker {

    /**
     * @brief Type identifier for the marker (e.g., "TEE", "TESSMAX", "TESS")
     */
    std::string type;    // TEE, TESSMAX, TESS, etc.

    /**
     * @brief Index position in the coordinate array where marker applies
     */
    int index;           // Index position in coordinate array

    /**
     * @brief Constructor creating a marker with type and index
     * @param type Marker type identifier string
     * @param index Array index position for the marker
     */
    AirfoilMarker(const std::string& type, int index) : type(type), index(index) {}

};