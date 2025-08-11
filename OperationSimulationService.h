#pragma once

#include <vector>
#include <memory>

#include "ITurbine.h"
#include "IAerodynamicAnalyzer.h"


// OperationSimulationService - responsible for running the operation simulation
class OperationSimulationService
{
private:

	std::vector<std::unique_ptr<ITurbine>> turbines;
	std::vector< std::unique_ptr<IAerodynamicAnalyzer>> analyzers;

public:

	void addTurbine(std::unique_ptr <ITurbine> turbine);
	void addAnalyzer(std::unique_ptr <IAerodynamicAnalyzer> analyzer);
	void runSimulation();

};

