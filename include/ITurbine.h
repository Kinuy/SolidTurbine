#pragma once

/**
 * @brief Interface for turbine configuration and properties
 *
 * ITurbine provides a common interface for different turbine types,
 * enabling polymorphic handling of various turbine configurations
 * through shared properties and characteristics.
 *
 * ## Key Features
 * - **Polymorphic Design**: Common interface for different turbine types
 * - **Configuration Access**: Provides access to fundamental turbine properties
 * - **Type Abstraction**: Enables generic turbine handling in simulation systems
 *
 * ## Common Implementations
 * - **HorizontalTurbine**: Horizontal axis wind turbines
 * - **VerticalTurbine**: Vertical axis wind turbines
 * - **MarineTurbine**: Underwater or tidal turbines
 *
 * @see HorizontalTurbine for concrete implementation
 * @see OperationSimulationService for usage in simulation systems
 * @see IAerodynamicAnalyzer for analysis interfaces that work with turbines
 *
 * @example
 * ```cpp
 * std::unique_ptr<ITurbine> turbine = std::make_unique<HorizontalTurbine>(3);
 * int bladeCount = turbine->getNumberOfBlades();
 *
 * // Polymorphic usage in collections
 * std::vector<std::unique_ptr<ITurbine>> turbines;
 * turbines.push_back(std::make_unique<HorizontalTurbine>(3));
 * ```
 */
class ITurbine {

public:

	/**
	* @brief Virtual destructor for proper cleanup of derived classes
	*/
	virtual ~ITurbine() = default;

	/**
	 * @brief Gets the number of blades for this turbine configuration
	 * @return Number of blades in the turbine design
	 */
	virtual int getNumberOfBlades() const = 0;
};