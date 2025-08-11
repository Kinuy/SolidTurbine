#pragma once

#include<string>
#include<vector>
#include<stdexcept>

// Responsible for blade geometry data structure
struct BladeGeometryRow {
    std::string type;           // "DEF"
    double bladeRadius;         // [m]
    double chord;               // [m] 
    double twist;               // [deg]
    double relativeThickness;   // [%]
    double xt4;                 // [m]
    double yt4;                 // [m]
    double pcbaX;               // [m]
    double pcbaY;               // [m]
    double relativeTwistAxis;   // [%]

    BladeGeometryRow() = default;
    BladeGeometryRow(const std::vector<std::string>& tokens) {
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
};