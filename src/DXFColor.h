#pragma once

/**
 * @brief Represents AutoCAD DXFColor Index (ACI) for entity coloring
 *
 * Encapsulates DXF color system using AutoCAD DXFColor Index values.
 * Standard colors: 1=Red, 2=Yellow, 3=Green, 4=Cyan, 5=Blue, 6=Magenta, 7=White/Black
 */
struct DXFColor {

    /**
	 * @brief AutoCAD DXFColor Index (1-255, 7=default white/black)
     */
    int colorNumber;

    /**
     * @brief Constructs a color object
     * @param num AutoCAD DXFColor Index number (default: 7 for white/black)
     */
    DXFColor(int num = 7) : colorNumber(num) {}

};