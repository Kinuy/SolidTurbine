#pragma once

//AirfoilGeometryPoint - responsible for storing one point of geometry data of an airfoil
struct AirfoilGeometryPoint {
	double xCoordinate;   // x coordinate
	double yCoordinate;   // y coordinate

	AirfoilGeometryPoint(const double xCoordinate, const double yCoordinate)
		: xCoordinate(xCoordinate), yCoordinate(yCoordinate){
	}
};								