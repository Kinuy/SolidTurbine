#pragma once
/**
 * @file IInductionModel.h
 * @brief Strategy interface for computing BEM induction factors.
 *
 * Open/Closed: the empirical wake-correction model, Glauert correction,
 * or any future model is plugged in without altering the Solver core.
 */
#include <cstddef>

struct InductionFactors
{
    double a_axi{0.0}; ///< axial induction factor
    double a_rot{0.0}; ///< tangential induction factor
};

struct InductionInput
{
    double k;     ///< axial loading parameter = sigma*cn / (4*F*sin²phi)
    double k_rot; ///< tangential loading param = sigma*ct / (4*F*sin*cos*phi)
    double phi;   ///< flow angle [rad]
    double F;     ///< combined loss factor
};

/**
 * @brief Abstract induction model.
 */
class IInductionModel
{
public:
    virtual ~IInductionModel() = default;

    /**
     * @brief Compute axial and tangential induction factors from k values.
     */
    virtual InductionFactors Compute(InductionInput const &in) const = 0;
};
