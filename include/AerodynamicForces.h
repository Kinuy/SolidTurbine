#pragma once

/**
 * @brief Data structure for aerodynamic forces and moments from turbine analysis
 *
 * AerodynamicForces contains the complete set of aerodynamic loads and performance
 * coefficients resulting from aerodynamic analysis of wind turbines. Includes both
 * dimensionless coefficients and dimensional forces/moments for comprehensive
 * performance evaluation and structural load assessment.
 *
 * ## Performance Coefficients (Dimensionless)
 * - **Cp**: Power coefficient representing power extraction efficiency
 * - **Ct**: Thrust coefficient representing axial loading
 * - **Cm**: Torque coefficient representing rotational moment
 *
 * ## Force Components [N]
 * - **Fx (Edge)**: In-plane tangential force (rotor rotation direction)
 * - **Fy (Flap)**: Out-of-plane force (perpendicular to rotor disc)
 * - **Fz**: Axial force (along wind direction, typically thrust)
 *
 * ## Moment Components [Nm]
 * - **Mx (Edge)**: Moment about x-axis (edge-wise bending)
 * - **My (Flap)**: Moment about y-axis (flap-wise bending)
 * - **Mz**: Moment about z-axis (torsional moment)
 *
 * ## Coordinate System Convention
 * Assumes standard wind turbine coordinate system where:
 * - X-axis: Tangential to rotor rotation (edge direction)
 * - Y-axis: Perpendicular to rotor disc (flap direction)
 * - Z-axis: Along wind direction (thrust direction)
 *
 * @see IAerodynamicAnalyzer for analysis methods producing these forces
 * @see OperationConditions for input conditions affecting force calculation
 *
 * @example
 * ```cpp
 * // Create forces from BEM analysis results
 * AerodynamicForces forces(0.45, 0.8, 0.025, 1250.0, 2800.0, 15000.0,
 *                         8500.0, 125000.0, 2200.0);
 *
 * // Access performance metrics
 * double efficiency = forces.powerCoeffiCp;  // 0.45 (45% efficiency)
 * double thrust = forces.Fz;                 // 15000 N axial thrust
 * ```
 */
struct AerodynamicForces {

	/**
	 * @brief Power coefficient Cp (dimensionless)
	 *
	 * Ratio of actual power extracted to theoretical maximum power available
	 * in the wind. Typical values: 0.35-0.50 for modern wind turbines.
	 */
	double powerCoeffiCp;	// Power coefficient Cp [-]

	/**
	 * @brief Thrust coefficient Ct (dimensionless)
	 *
	 * Ratio of thrust force to dynamic pressure times rotor disc area.
	 * Related to axial induction and wake development.
	 */
	double thrustCoeffCt;	// Thrust coefficient Ct [-]

	/**
	 * @brief Torque coefficient Cm (dimensionless)
	 *
	 * Ratio of rotor torque to dynamic pressure, rotor disc area, and radius.
	 * Directly related to power coefficient through tip speed ratio.
	 */
	double torqueCoeffCm;	// Torque coefficient Cm [-]

	/**
	 * @brief Edge force in x-direction [N]
	 *
	 * Tangential force component in the rotor plane, typically associated
	 * with centrifugal and gyroscopic effects on the blade.
	 */
	double edgeForceFx;		// Edge force in x direction Fx [N]

	/**
	 * @brief Flap force in y-direction [N]
	 *
	 * Out-of-plane force component perpendicular to rotor disc, primarily
	 * from aerodynamic lift and gravitational effects.
	 */
	double flapForceFy;		// Flap force in y direction Fy [N]

	/**
	 * @brief Force in z-direction [N]
	 *
	 * Axial force component along wind direction, typically representing
	 * thrust force on the rotor and support structure.
	 */
	double Fz;				// Force in z direction Fz [N]

	/**
	 * @brief Edge moment about x-axis [Nm]
	 *
	 * Bending moment in the edge-wise direction, typically smaller than
	 * flap moments but important for fatigue analysis.
	 */
	double edgeMomentMx;	// Edge moment in x direction Mx [Nm]

	/**
	 * @brief Flap moment about y-axis [Nm]
	 *
	 * Bending moment in the flap-wise direction, typically the dominant
	 * load for blade structural design and tower clearance.
	 */
	double flapMomentMy;	// Flap moment in y direction My [Nm]

	/**
	 * @brief Moment about z-axis [Nm]
	 *
	 * Torsional moment about the blade axis, important for blade twist
	 * and pitch bearing loads.
	 */
	double Mz;				// Moment in z direction Mz [Nm]

	/**
	 * @brief Constructor with default zero values for all force components
	 * @param powerCoeffiCp Power coefficient Cp (default: 0.0)
	 * @param thrustCoeffCt Thrust coefficient Ct (default: 0.0)
	 * @param torqueCoeffCm Torque coefficient Cm (default: 0.0)
	 * @param edgeForceFx Edge force Fx [N] (default: 0.0)
	 * @param flapForceFy Flap force Fy [N] (default: 0.0)
	 * @param Fz Axial force Fz [N] (default: 0.0)
	 * @param edgeMomentMx Edge moment Mx [Nm] (default: 0.0)
	 * @param flapMomentMy Flap moment My [Nm] (default: 0.0)
	 * @param Mz Torsional moment Mz [Nm] (default: 0.0)
	 */
	AerodynamicForces(
		const double powerCoeffiCp = 0.0, 
		const double thrustCoeffCt = 0.0, 
		const double torqueCoeffCm = 0.0, 
		const double edgeForceFx = 0.0, 
		const double flapForceFy = 0.0, 
		const double Fz = 0.0, 
		const double edgeMomentMx = 0.0, 
		const double flapMomentMy = 0.0, 
		const double Mz = 0.0)
		: 
		powerCoeffiCp(powerCoeffiCp), 
		thrustCoeffCt(thrustCoeffCt), 
		torqueCoeffCm(torqueCoeffCm), 
		edgeForceFx(edgeForceFx), 
		flapForceFy(flapForceFy), 
		Fz(Fz), 
		edgeMomentMx(edgeMomentMx), 
		flapMomentMy(flapMomentMy), 
		Mz(Mz) {
	}

};
