#pragma once

#include <vector>

// OperationConditions - responsible for storing turbine operations data
struct OperationConditions {
	std::vector<double> windSpeeds;	// wind speeds vector in m/s
	std::vector<double> tipSpeedRatios; // tip speed ratios vector : windSpeeds / rotationalSpeeds
	std::vector<double> pitchAngles; // pitch angles vector in degrees

	OperationConditions(std::vector<double> windSpeeds , std::vector<double> tipSpeedRatios , std::vector<double> pitchAngles):
		windSpeeds(windSpeeds), 
		tipSpeedRatios(tipSpeedRatios), 
		pitchAngles(pitchAngles) {
	}
};
