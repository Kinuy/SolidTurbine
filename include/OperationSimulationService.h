#pragma once

#include <vector>
#include <memory>

#include "ITurbine.h"
#include "IAerodynamicAnalyzer.h"


/**
 * @brief Service for managing and executing aerodynamic simulation workflows
 *
 * OperationSimulationService coordinates aerodynamic analysis by managing
 * collections of turbines and analyzers. Enables batch simulation execution
 * across multiple turbine configurations and analysis methods.
 *
 * ## Key Features
 * - **Multi-Turbine Support**: Manages collections of different turbine types
 * - **Multi-Analyzer Support**: Supports various aerodynamic analysis methods
 * - **Batch Processing**: Executes all turbine-analyzer combinations
 * - **Ownership Management**: Takes ownership of turbines and analyzers
 *
 * ## Typical Workflow
 * 1. Add turbines and analyzers to collections
 * 2. Configure operation and environmental conditions
 * 3. Execute simulation across all combinations
 * 4. Retrieve and process analysis results
 *
 * @see ITurbine for turbine interface
 * @see IAerodynamicAnalyzer for analyzer interface
 *
 * @example
 * ```cpp
 * OperationSimulationService service;
 * service.addTurbine(std::make_unique<HorizontalTurbine>(3));
 * service.addAnalyzer(std::make_unique<BEMAnalyzer>());
 * service.runSimulation();
 * ```
 */
class OperationSimulationService{

private:

	/**
	 * @brief Collection of turbines for simulation analysis
	 */
	std::vector<std::unique_ptr<ITurbine>> turbines;

	/**
	 * @brief Collection of aerodynamic analyzers for performance evaluation
	 */
	std::vector< std::unique_ptr<IAerodynamicAnalyzer>> analyzers;

public:

	/**
	 * @brief Adds a turbine to the simulation collection
	 * @param turbine Unique pointer to turbine (ownership transferred)
	 * @note Null pointers are ignored silently
	 */
	void addTurbine(std::unique_ptr <ITurbine> turbine);

	/**
	 * @brief Adds an aerodynamic analyzer to the simulation collection
	 * @param analyzer Unique pointer to analyzer (ownership transferred)
	 * @note Null pointers are ignored silently
	 */
	void addAnalyzer(std::unique_ptr <IAerodynamicAnalyzer> analyzer);

	/**
	 * @brief Executes simulation for all turbine-analyzer combinations
	 *
	 * Runs aerodynamic analysis for each turbine using each analyzer with
	 * predefined operation and environmental conditions.
	 *
	 * @note Uses hardcoded example conditions - should be made configurable
	 */
	void runSimulation();

};

