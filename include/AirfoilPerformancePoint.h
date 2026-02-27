#pragma once

/**
 * @brief Data point representing airfoil aerodynamic performance at specific angle of attack
 *
 * AirfoilPerformancePoint stores the fundamental aerodynamic coefficients
 * for an airfoil at a single operating condition. Used to build performance
 * curves and lookup tables for aerodynamic analysis.
 *
 * ## Coefficient Definitions
 * - **cl**: Lift coefficient (force perpendicular to flow direction)
 * - **cd**: Drag coefficient (force parallel to flow direction)
 * - **cm**: Moment coefficient (pitching moment about reference point)
 *
 * ## Typical Usage
 * Multiple points form complete performance curves across angle of attack
 * range for aerodynamic interpolation and analysis calculations.
 *
 * @see AirfoilPerformanceData for collections of performance points
 *
 * @example
 * ```cpp
 * AirfoilPerformancePoint point(5.0, 0.8, 0.02, -0.1);
 * // 5° angle of attack, Cl=0.8, Cd=0.02, Cm=-0.1
 * ```
 */
struct AirfoilPerformancePoint {

    /**
     * @brief Angle of attack [degrees]
     */
    double alpha;  // Angle of attack [degrees]

    /**
     * @brief Lift coefficient (dimensionless)
     */
    double cl;     // Lift coefficient

    /**
     * @brief Drag coefficient (dimensionless)
     */
    double cd;     // Drag coefficient  

    /**
     * @brief Moment coefficient (dimensionless, optional)
     */
    double cm;     // Moment coefficient (optional)

    /**
     * @brief Constructor initializing all aerodynamic coefficients
     * @param alpha Angle of attack [degrees]
     * @param cl Lift coefficient
     * @param cd Drag coefficient
     * @param cm Moment coefficient
     */
    AirfoilPerformancePoint(double alpha, double cl, double cd, double cm)
        : alpha(alpha), cl(cl), cd(cd), cm(cm) {
    }

};
