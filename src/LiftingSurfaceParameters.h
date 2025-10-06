#pragma once

#include <limits>

/**
 * @brief Parameters defining the characteristics of a lifting surface calculation
 */
struct LiftingSurfaceParameters {
	static int    linearSolverMaxIterations;

	/**
	* @brief
	*/
	static double linearSolverTolerance;

	/**
	* @brief
	*/
	static bool   unsteadyBernoulli;

	/**
	* @brief
	*/
	static bool   convectWake;

	/**
	* @brief
	*/
	static bool   wakeEmissionFollowBisector;

	/**
	* @brief
	*/
	static double wakeEmissionDistanceFactor;

	/**
	* @brief
	*/
	static double wakeVortexCoreRadius;

	/**
	* @brief
	*/
	static double staticWakeLength;

	/**
	* @brief
	*/
	static double zeroThreshold;

	/**
	* @brief
	*/
	static double rlCollocationPointDelta;

	/**
	* @brief
	*/
	static double rlInterpolationLayerThickness;

	/**
	* @brief
	*/
	static int    rlMaxBoundaryLayerIterations;

	/**
	* @brief
	*/
	static double rlBoundaryLayerIterationTolerance;

	/**
	* @brief
	*/
	static double rlFluidKinematicViscosity;

	/**
	* @brief
	*/
	static double rlInitialVortexCoreRadius;

	/**
	* @brief
	*/
	static double rlMinVortexCoreRadius;

	/**
	* @brief
	*/
	static double rlLambsConstant;

	/**
	* @brief
	*/
	static double rlAPrime;

	/**
	* @brief
	*/
	typedef struct {
		double vortex_reynolds_number;
		double a_1;
		double b_1;
		double a_2;
		double b_2;
		double b_3;
	} ramasamy_leishman_data_row;

	/**
	* @brief
	*/
	static ramasamy_leishman_data_row ramasamy_leishman_data[12];

};
