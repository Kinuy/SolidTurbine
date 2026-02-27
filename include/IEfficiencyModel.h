#pragma once
/**
 * @file IEfficiencyModel.h
 * @brief Strategy interface for drivetrain / generator efficiency.
 *
 * Single Responsibility: owns only the η = f(P_mech) mapping.
 * Open/Closed: the Harris generator model, table-lookup model, or a fixed
 * constant can all implement this interface without touching the controller.
 */

class IEfficiencyModel
{
public:
    virtual ~IEfficiencyModel() = default;

    /**
     * @brief Return drivetrain efficiency η ∈ (0, 1] for mechanical power.
     * @param p_mech  Aerodynamic (mechanical) power at shaft [W].
     */
    virtual double Efficiency(double p_mech) const = 0;
};
