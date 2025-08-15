#pragma once
#include "EnvironmentalConditions.h"
#include "OperationConditions.h"

class IAerodynamicAnalyzer {
	public:

	virtual ~IAerodynamicAnalyzer() = default;

	// Method to analyze aerodynamic performance of a turbine
	virtual void analyzePerformance() = 0;

	// Method to get the results of the aerodynamic analysis
	virtual void getAnalysisResults() const = 0;

	// Method to set environmental conditions for the analysis
	virtual void setEnvironmentalConditions(const EnvironmentalConditions& conditions) = 0;

	// Method to set operation conditions for the analysis
	virtual void setOperationConditions(const OperationConditions& conditions) = 0;
};