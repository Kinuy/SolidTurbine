#pragma once

/**
 * @brief Represents a 3D point in Cartesian coordinate system
 *
 * Simple value object for storing 3D coordinates used throughout the DXF
 * export system. Follows standard CAD coordinate conventions.
 */
struct DXFPoint3D {

    /**
     * @brief X, Y, Z coordinates in drawing units
     */
    double x, y, z;

    /**
     * @brief Constructs a 3D point
     * @param x X-coordinate (default: 0.0)
     * @param y Y-coordinate (default: 0.0)
     * @param z Z-coordinate (default: 0.0)
     */
    DXFPoint3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};