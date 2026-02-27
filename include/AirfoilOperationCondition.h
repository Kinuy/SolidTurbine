#pragma once

// #include "Angles.h"

/**
 * @brief Airfoil operating conditions for polar data lookup
 */
struct AirfoilOperationCondition
{

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
    AirfoilOperationCondition(double re, double ma, double aoa)
        : reynolds(re), mach(ma), alpha(aoa)
    {
    }
};