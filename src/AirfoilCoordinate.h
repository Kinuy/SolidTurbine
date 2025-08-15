#pragma once


// Individual Airfoil Geometry Data Structure
struct AirfoilCoordinate {
    double x;
    double y;
	double z; // Optional z-coordinate for 3D representation

    AirfoilCoordinate(double xVal, double yVal, double zVal=0) : x(xVal), y(yVal), z(zVal) {}

    bool is2D() const { return z == 0.0; }
    
    bool is3D() const { return z != 0.0; }

};