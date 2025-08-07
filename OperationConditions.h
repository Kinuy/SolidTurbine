#pragma once

#include <vector>

// OperationConditions - responsible for storing turbine operations data
struct OperationConditions {
	std::vector<double> windSpeeds;
	std::vector<double> tipSpeedRatios;
	std::vector<double> pitchAngles;

	OperationConditions(std::vector<double> windSpeeds , std::vector<double> tipSpeedRatios , std::vector<double> pitchAngles):
		windSpeeds(windSpeeds), 
		tipSpeedRatios(tipSpeedRatios), 
		pitchAngles(pitchAngles) {
	}
};
