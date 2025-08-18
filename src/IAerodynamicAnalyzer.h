#pragma once
#include "EnvironmentalConditions.h"
#include "OperationConditions.h"

/**
 * @brief Interface for aerodynamic analysis engines
 *
 * IAerodynamicAnalyzer defines the contract for aerodynamic performance
 * analysis of turbine systems. Provides a standardized interface for
 * different analysis methods and computational approaches.
 *
 * ## Analysis Workflow
 * 1. Set environmental conditions (air density, temperature, etc.)
 * 2. Set operational conditions (RPM, pitch angles, etc.)
 * 3. Execute performance analysis
 * 4. Retrieve analysis results
 *
 * ## Key Features
 * - **Configurable Conditions**: Separate environmental and operational parameters
 * - **Flexible Implementation**: Strategy pattern for different analysis methods
 * - **Results Access**: Structured result retrieval interface
 *
 * @see EnvironmentalConditions for atmospheric parameter definitions
 * @see OperationConditions for turbine operational parameter definitions
 *
 * @example
 * ```cpp
 * std::unique_ptr<IAerodynamicAnalyzer> analyzer =
 *     std::make_unique<BEMAnalyzer>();
 *
 * analyzer->setEnvironmentalConditions(envConditions);
 * analyzer->setOperationConditions(opConditions);
 * analyzer->analyzePerformance();
 * analyzer->getAnalysisResults();
 * ```
 */
class IAerodynamicAnalyzer {

	public:

	/**
	* @brief Virtual destructor for proper cleanup of derived classes
	*/
	virtual ~IAerodynamicAnalyzer() = default;

	/**
	 * @brief Executes aerodynamic performance analysis
	 *
	 * Performs the main computational analysis using configured environmental
	 * and operational conditions. Implementation depends on specific analysis
	 * method (BEM, CFD, etc.).
	 *
	 * @note Must be called after setting both environmental and operational conditions
	 */
	virtual void analyzePerformance() = 0;

	/**
	 * @brief Retrieves results from the completed analysis
	 *
	 * Provides access to computed aerodynamic performance data such as
	 * power output, thrust, torque, and efficiency metrics.
	 *
	 * @note Should only be called after successful completion of analyzePerformance()
	 */
	virtual void getAnalysisResults() const = 0;

	/**
	 * @brief Sets environmental conditions for analysis
	 *
	 * Configures atmospheric and environmental parameters that affect
	 * aerodynamic performance (air density, temperature, viscosity, etc.).
	 *
	 * @param conditions Environmental conditions structure
	 */
	virtual void setEnvironmentalConditions(const EnvironmentalConditions& conditions) = 0;

	/**
	 * @brief Sets operational conditions for analysis
	 *
	 * Configures turbine operational parameters such as rotational speed,
	 * blade pitch angles, and control settings.
	 *
	 * @param conditions Operational conditions structure
	 */
	virtual void setOperationConditions(const OperationConditions& conditions) = 0;
};