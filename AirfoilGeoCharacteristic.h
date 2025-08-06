#pragma once

//AirfoilGeoCharacteristics - responsible for storing geometry data of an airfoil
struct AirfoilGeoCharacteristics {
	double xCoordinate;   // x coordinate
	double yCoordinate;   // y coordinateS

	AirfoilGeoCharacteristics(const double xCoordinate, const double yCoordinate)
		: xCoordinate(xCoordinate), yCoordinate(yCoordinate){
	}
};