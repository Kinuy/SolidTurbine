#pragma once
/**
 * @file ILossModel.h
 * @brief Strategy interface for aerodynamic loss models (tip loss, hub loss).
 *
 * Open/Closed Principle: new loss models (e.g. Shen, Lindenburg) can be added by
 * implementing this interface without touching the Solver.
 *
 * Interface Segregation: callers only ever need Evaluate().
 */
#include <cstddef>

/**
 * @brief Minimal data the loss model needs from the current solve state.
 */
struct LossModelInput
{
    double radius;       ///< local blade radius [m]
    double rotor_radius; ///< rotor tip radius [m]
    double hub_radius;   ///< hub radius [m]
    double phi;          ///< flow angle [rad]
    double num_blades;
    double chord; ///< chord at outermost/innermost section (singularity avoider)
    double tip_extra_distance{0.0};
};

/**
 * @brief Abstract loss model.
 */
class ILossModel
{
public:
    virtual ~ILossModel() = default;

    /**
     * @brief Compute the loss factor F ∈ [0,1].
     * @param input  Current aerodynamic state for this section.
     * @return       Prandtl-style loss factor.
     */
    virtual double Evaluate(LossModelInput const &input) const = 0;
};
