#pragma once

/**
 * @brief Represents a 3D point in Cartesian coordinate system
 *
 * Simple value object for storing 3D coordinates used throughout the DXF
 * export system. Follows standard CAD coordinate conventions.
 */
struct DXFPoint3D {
    /** @brief X, Y, Z coordinates in drawing units */
    double x, y, z;

    /**
     * @brief Constructs a 3D point
     * @param x X-coordinate (default: 0.0)
     * @param y Y-coordinate (default: 0.0)
     * @param z Z-coordinate (default: 0.0)
     */
    DXFPoint3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};


/**
 * @brief Represents AutoCAD Color Index (ACI) for entity coloring
 *
 * Encapsulates DXF color system using AutoCAD Color Index values.
 * Standard colors: 1=Red, 2=Yellow, 3=Green, 4=Cyan, 5=Blue, 6=Magenta, 7=White/Black
 */
struct DXFColor {
    /** @brief AutoCAD Color Index (1-255, 7=default white/black) */
    int colorNumber;

    /**
     * @brief Constructs a color object
     * @param num AutoCAD Color Index number (default: 7 for white/black)
     */
    DXFColor(int num = 7) : colorNumber(num) {}
};
