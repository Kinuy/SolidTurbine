#pragma once


/**
 * @brief Airfoil operating conditions for polar data lookup
 */
struct AirfoilOperationCondition {

    /**
     * @brief Reynolds number for the flow conditions
	 */
    double reynolds;

    /**
	 * @brief Mach number for the flow conditions
	 */
    double mach;

	/**
	* @brief Angle of attack in degrees
    */
    double alpha;

    /**
     * @brief Default constructor
	 * @param ao Angle of attack in degrees
	 * @param re Reynolds number
	 * @param ma Mach number
	 */
    AirfoilOperationCondition(double aoa, double re, double ma=0.3)
        : alpha(aoa), reynolds(re), mach(ma){
    }
};