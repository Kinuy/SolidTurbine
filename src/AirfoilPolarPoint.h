#pragma once
#include "AirfoilOperationCondition.h"
#include "AirfoilAeroCoefficients.h"


/**
 * @brief Represents a single point in the airfoil polar data
 */
struct AirfoilPolarPoint {
    /**
     * @brief Operating conditions for this polar point
	 */
    AirfoilOperationCondition condition;

    /**
	 * @brief Aerodynamic coefficients for this polar point
	 */
    AirfoilAeroCoefficients coefficients;

    /**
     * @brief Default constructor
	 * @param cond Operating conditions for this polar point
	 * @param coeffs Aerodynamic coefficients for this polar point
	 */
    AirfoilPolarPoint(const AirfoilOperationCondition& cond, const AirfoilAeroCoefficients& coeffs)
        : condition(cond), coefficients(coeffs) {
    }
};