#pragma once
/**
 * @file ITurbineController.h
 * @brief Abstract interface for turbine operating-point control strategies.
 *
 * Dependency Inversion Principle: opermot depends on this abstraction.
 * Open/Closed Principle: new control strategies (e.g. derating, storm control,
 * density-corrected operation) are added by creating a new class, not by
 * modifying existing ones.
 */
#include <string>

/**
 * @brief The operating state the controller must resolve.
 */
struct ControllerInput
{
    double vinf; ///< free-stream wind speed [m/s]
    double Pel;  ///< current electrical power (may be from previous iter) [W]
};

/**
 * @brief The operating-point the controller returns.
 */
struct ControllerOutput
{
    double vtip;   ///< blade tip speed [m/s]
    double n;      ///< rotor speed [rpm]
    double pitch;  ///< collective pitch [deg]
    double lambda; ///< tip-speed ratio [-]
};

/**
 * @brief Pure interface for all turbine controllers.
 */
class ITurbineController
{
public:
    virtual ~ITurbineController() = default;

    /**
     * @brief Given a wind speed and current power, compute the desired
     *        operating point (vtip, n, pitch).
     */
    virtual ControllerOutput ComputeOperatingPoint(ControllerInput const &in) const = 0;

    /**
     * @brief Identifier string for logging.
     */
    virtual std::string Name() const = 0;
};
