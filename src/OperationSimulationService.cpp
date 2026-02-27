#include "OperationSimulationService.h"

void OperationSimulationService::addTurbine(std::unique_ptr<ITurbine> turbine)
{
	if (turbine) {
		turbines.push_back(std::move(turbine));
	}
}

void OperationSimulationService::addAnalyzer(std::unique_ptr <IAerodynamicAnalyzer> analyzer)
{
	if (analyzer) {
		analyzers.push_back(std::move(analyzer));
	}
}

void OperationSimulationService::runSimulation()
{
	for ([[maybe_unused]] const auto &turbine : turbines)
	{
		for (const auto & analyzer : analyzers) {
			analyzer->setOperationConditions(OperationConditions({ 10.0 }, { 7.0 }, { 5.0 })); // Example conditions
			analyzer->setEnvironmentalConditions(EnvironmentalConditions());
			analyzer->analyzePerformance();
			analyzer->getAnalysisResults();
		}
	}
}
