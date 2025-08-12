#pragma once


// Individual Airfoil Geometry Data Structure
struct AirfoilCoordinate {
    double x;
    double y;

    AirfoilCoordinate(double xVal, double yVal) : x(xVal), y(yVal) {}
};