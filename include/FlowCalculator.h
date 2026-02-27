#pragma once
/**
 * @file FlowCalculator.h
 * @brief Refactored FlowCalculator — pure orchestrator of flow-field computation.
 *
 * SOLID compliance:
 *  S – Responsible only for assembling the global and local velocity fields
 *      from its injected components.  It does NOT implement any shear formula,
 *      veer formula, or coordinate transform.
 *  O – New atmospheric models are injected; this class never changes for them.
 *  L – Satisfies the same public API as the original FlowCalculator so all
 *      call sites (NingSolver, postprocess…) compile unchanged.
 *  I – Exposes only what consumers need: LocalLambda(), BladeLocalVelocities(),
 *      v_inf(), Lambda().
 *  D – Depends on IShearModel, IVeerModel, IInletVelocityProvider,
 *      ICoordinateTransformer — abstractions, not concrete classes.
 *
 * Migration note
 * ──────────────
 * The original FlowCalculator accepted a FlowModifiers struct and built
 * everything internally.  The refactored version accepts injected strategies
 * instead.  FlowCalculatorFactory (see below) rebuilds the original behaviour
 * from a FlowModifiers struct so existing call sites need only swap to the
 * factory call.
 */
#include <cstddef>
#include <float.h>
#include <memory>
#include <vector>

#include "ICoordinateTransformer.h"
#include "IInletVelocityProvider.h"
#include "IShearModel.h"
#include "IVeerModel.h"
#include "TurbineGeometry.h"
#include "MathUtilities.h"

class FlowCalculator
{
public:
    // ── Construction ─────────────────────────────────────────────────────────
    /**
     * @brief Construct with full dependency injection.
     *
     * @param geometry       Non-owning pointer to turbine geometry.
     * @param rot_rate       Rotor angular velocity [rad/s].
     * @param psi            Azimuth angle [rad].
     * @param inlet          Injected inlet velocity provider (uniform or TurbSim).
     * @param shear          Injected shear model.
     * @param veer           Injected veer model.
     * @param transformer    Injected coordinate transformer.
     */
    FlowCalculator(TurbineGeometry const *geometry,
                   double rot_rate,
                   double psi,
                   IInletVelocityProvider const *inlet,
                   IShearModel const *shear,
                   IVeerModel const *veer,
                   ICoordinateTransformer const *transformer);

    // Non-copyable (large velocity arrays + non-owning pointers).
    FlowCalculator(FlowCalculator const &) = delete;
    FlowCalculator &operator=(FlowCalculator const &) = delete;
    FlowCalculator(FlowCalculator &&) = default;
    FlowCalculator &operator=(FlowCalculator &&) = default;

    // ── Public interface (unchanged from original) ────────────────────────────

    /**
     * @brief Local tip-speed ratio for a single blade section.
     *
     * λ_loc = v_tangential / v_axial.  Returns DBL_EPSILON if near-zero
     * to avoid division by zero in the BEM solver.
     */
    double LocalLambda(std::size_t section) const;

    /**
     * @brief Return axial and tangential local velocities for a section.
     *
     * @param section    Radial section index.
     * @param axial_vel  Output: axial velocity component [m/s].
     * @param tangt_vel  Output: tangential velocity component [m/s].
     */
    void BladeLocalVelocities(std::size_t section,
                              double *axial_vel,
                              double *tangt_vel) const;

    /// Hub-height free-stream velocity [m/s].
    double v_inf() const { return v_inf_; }

    /// Rotor tip-speed ratio [-].
    double Lambda() const;

    /// Local tip-speed ratio at a section [−].
    double LocalLambdaAt(std::size_t section) const { return LocalLambda(section); }

private:
    // ── Injected dependencies (non-owning) ───────────────────────────────────
    TurbineGeometry const *geometry_;
    IInletVelocityProvider const *inlet_;
    IShearModel const *shear_;
    IVeerModel const *veer_;
    ICoordinateTransformer const *transformer_;

    // ── Scalar state ──────────────────────────────────────────────────────────
    double rot_rate_;
    double psi_;
    double v_inf_;

    // ── Computed velocity fields ──────────────────────────────────────────────
    std::vector<WVPMUtilities::Vec3D<double>> global_vels_;
    std::vector<WVPMUtilities::Vec3D<double>> local_vels_;

    // ── Private orchestration steps ───────────────────────────────────────────

    /// Step 1 – populate global_vels_ from the inlet provider.
    void BuildInletField();

    /// Step 2 – apply shear modification to global_vels_.
    void ApplyShear();

    /// Step 3 – apply veer modification to global_vels_.
    void ApplyVeer();

    /// Step 4 – convert to local frame and add rotational velocity.
    void BuildLocalField();

    /// Compute the rotational velocity contribution per section [m/s].
    std::vector<double> RotationalVelocities() const;
};
