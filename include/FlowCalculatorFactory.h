#pragma once
/**
 * @file FlowCalculatorFactory.h
 * @brief Factory that constructs a FlowCalculator from the legacy FlowModifiers
 *        struct, preserving full backward compatibility for existing call sites.
 *
 * Migration path
 * ──────────────
 * Old code:
 *   FlowCalculator fc(&geometry, rot_rate, vinf, psi, flow_modifiers);
 *
 * New code (drop-in):
 *   FlowCalculatorFactory factory;
 *   auto fc = factory.Build(&geometry, rot_rate, vinf, psi, flow_modifiers);
 *
 * The factory owns all concrete strategy objects it creates; the FlowCalculator
 * borrows non-owning const pointers.  The factory must therefore outlive the
 * FlowCalculator — which is the same lifetime as the original local variable.
 *
 * SOLID compliance:
 *  S – Owns only the wiring of concrete strategies.
 *  O – Adding a new shear model: implement IShearModel and call Build directly
 *      with the new object — the factory is unaffected.
 *  D – Creates concretes internally; FlowCalculator sees only the interfaces.
 */
#include <memory>
#include <stdexcept>

#include "FlowCalculator.h"
#include "InletVelocityProviders.h"
#include "ShearModels.h"
#include "VeerModels.h"
#include "PsiCoordinateTransformer.h"
#include "FlowModifiers.h" // FlowModifiers — existing struct, unchanged

class FlowCalculatorFactory
{
public:
    /**
     * @brief Build a FlowCalculator from a FlowModifiers struct (legacy path).
     *
     * Selects the appropriate concrete strategy objects based on the flags
     * inside fm (shear.first, veer.first, etc.) and injects them.
     */
    std::unique_ptr<FlowCalculator> Build(
        TurbineGeometry const *geometry,
        double rot_rate,
        double v_inf,
        double psi,
        FlowModifiers const &fm)
    {
        // ── Inlet provider ────────────────────────────────────────────────────
        auto inlet = std::make_unique<UniformInletProvider>(v_inf);

        // ── Shear model ───────────────────────────────────────────────────────
        std::unique_ptr<IShearModel> shear;
        if (!fm.shear.first)
        {
            // Shear disabled: return v_inf unchanged at every height.
            shear = MakeNoShear(v_inf);
        }
        else
        {
            switch (fm.shear.second)
            {
            case 1:
                shear = std::make_unique<LogShearModel>(
                    fm.surface_roughness,
                    fm.ref_vel, fm.ref_height, fm.use_ref_hv);
                break;
            case 2:
                shear = std::make_unique<PowerLawShearModel>(
                    fm.shear_exponent,
                    fm.ref_vel, fm.ref_height, fm.use_ref_hv);
                break;
            case 3:
                shear = std::make_unique<DiabaticShearModel>(
                    fm.surface_roughness, fm.obukhov_length);
                break;
            default:
                throw std::invalid_argument(
                    "FlowCalculatorFactory: unknown shear mode " + std::to_string(fm.shear.second));
            }
        }

        // ── Veer model ────────────────────────────────────────────────────────
        std::unique_ptr<IVeerModel> veer;
        if (!fm.veer.first)
        {
            veer = std::make_unique<NoVeer>();
        }
        else
        {
            veer = std::make_unique<LinearVeerModel>(
                fm.veer.second, geometry->RotorRadius());
        }

        // ── Coordinate transformer ────────────────────────────────────────────
        auto transformer = std::make_unique<PsiCoordinateTransformer>(geometry, psi);

        // ── Store ownership ───────────────────────────────────────────────────
        owned_inlets_.push_back(std::move(inlet));
        owned_shears_.push_back(std::move(shear));
        owned_veers_.push_back(std::move(veer));
        owned_transformers_.push_back(std::move(transformer));

        return std::make_unique<FlowCalculator>(
            geometry,
            rot_rate,
            psi,
            owned_inlets_.back().get(),
            owned_shears_.back().get(),
            owned_veers_.back().get(),
            owned_transformers_.back().get());
    }

    /**
     * @brief Build a TurbSim-driven FlowCalculator.
     *
     * Replaces the old FlowCalculator::UseTurbsim() call pattern.
     * The factory creates a new FlowCalculator that reads velocities from tsm.
     */
    std::unique_ptr<FlowCalculator> BuildTurbSim(
        TurbineGeometry const *geometry,
        double rot_rate,
        double psi,
        TurbSimManager const *tsm,
        unsigned iteration,
        FlowModifiers const & /*fm*/)
    {
        auto inlet = std::make_unique<TurbSimInletProvider>(tsm, iteration);
        auto shear = MakeNoShear(tsm->hub_velocity()); // TurbSim already has profiles
        auto veer = std::make_unique<NoVeer>();        // ditto
        auto transformer = std::make_unique<PsiCoordinateTransformer>(geometry, psi);

        owned_inlets_.push_back(std::move(inlet));
        owned_shears_.push_back(std::move(shear));
        owned_veers_.push_back(std::move(veer));
        owned_transformers_.push_back(std::move(transformer));

        return std::make_unique<FlowCalculator>(
            geometry,
            rot_rate,
            psi,
            owned_inlets_.back().get(),
            owned_shears_.back().get(),
            owned_veers_.back().get(),
            owned_transformers_.back().get());
    }

private:
    // Factory owns all objects it creates; FlowCalculator borrows raw ptrs.
    std::vector<std::unique_ptr<IInletVelocityProvider>> owned_inlets_;
    std::vector<std::unique_ptr<IShearModel>> owned_shears_;
    std::vector<std::unique_ptr<IVeerModel>> owned_veers_;
    std::vector<std::unique_ptr<ICoordinateTransformer>> owned_transformers_;

    // Helper: "no shear" returns v_inf at every height (identity transform).
    static std::unique_ptr<IShearModel> MakeNoShear(double v_inf)
    {
        // A constant-velocity shear model: v(z) = v_inf always.
        // Implemented as PowerLaw with exponent=0 → (z/z_ref)^0 = 1.
        return std::make_unique<PowerLawShearModel>(
            0.0, v_inf, 1.0, true); // exponent=0, ref_vel=v_inf, use_ref_hv=true
    }
};
