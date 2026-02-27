#pragma once
/**
 * @file BEMPostprocessor.h
 * @brief Concrete BEM postprocessor.
 *
 * Computes all local and integrated aerodynamic quantities from a completed
 * NingSolver result. Replaces and modernises the legacy wvp::PostProcess class.
 *
 * SOLID compliance:
 *  S – responsible only for postprocessing; solve loop stays in NingSolver.
 *  O – extended via IBEMPostprocessor without modifying this class.
 *  L – fully satisfies IBEMPostprocessor.
 *  I – consumers see only IBEMPostprocessor.
 *  D – depends on TurbineGeometry, ISimulationConfig, FlowCalculator abstractions.
 *
 * Inputs:
 *  - SolverResult      (phi, a_ind_axi, a_ind_rot, cp, ct per section)
 *  - TurbineGeometry   (chord, radius, twist, aero_centre_x/y per section)
 *  - ISimulationConfig (air_density)
 *  - FlowCalculator    (local axial + tangential velocities per section)
 *  - NingSolver        (LocalFlowVel, LocalReynoldsNumber, LocalMachNumber)
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include "IBEMPostprocessor.h"
#include "IBEMSolver.h"
#include "ISimulationConfig.h"

// Forward declarations
class TurbineGeometry;
class FlowCalculator;

// ─────────────────────────────────────────────────────────────────────────────
/**
 * @brief All results produced by BEMPostprocessor::Process().
 *
 * Scalars are rotor-integrated quantities.
 * Vectors have one entry per blade section.
 */
struct BEMPostprocessResult
{
    // ── Rotor-integrated ─────────────────────────────────────────────────────
    double cp{0.0};      ///< aerodynamic power coefficient [-]
    double ct{0.0};      ///< thrust coefficient [-]
    double p{0.0};       ///< aerodynamic power [W]
    double thrust{0.0};  ///< rotor thrust force T [N]
    double torque{0.0};  ///< rotor torque Q [Nm]
    double ctorque{0.0}; ///< torque coefficient [-]
    double sum_fy{0.0};  ///< total in-plane (tangential) force [N]

    // ── Blade root moments (single blade) ────────────────────────────────────
    double mx{0.0}; ///< flapwise root moment [Nm]
    double my{0.0}; ///< edgewise root moment [Nm]
    double mz{0.0}; ///< torsional root moment [Nm]

    // ── Per-section local quantities ─────────────────────────────────────────
    std::vector<double> alpha_eff;          ///< effective AoA [rad]
    std::vector<double> cl;                 ///< lift coefficient per section [-]
    std::vector<double> cd;                 ///< drag coefficient per section [-]
    std::vector<double> cm;                 ///< moment coefficient per section [-]
    std::vector<double> cp_loc;             ///< local power coefficient [-]
    std::vector<double> ct_loc;             ///< local thrust coefficient [-]
    std::vector<double> element_length;     ///< dr per section [m]
    std::vector<double> element_thrust;     ///< dT per section [N]
    std::vector<double> element_torque;     ///< dQ per section [Nm]
    std::vector<double> element_fy;         ///< in-plane force per section [N]
    std::vector<double> element_mz;         ///< section torsion moment [Nm]
    std::vector<double> element_airfoil_moment; ///< airfoil pitching moment [Nm]

    // ── Integrated loads from tip to each section (blade beam loads) ─────────
    std::vector<double> integral_fx; ///< cumulative thrust  (tip→r) [N]
    std::vector<double> integral_fy; ///< cumulative in-plane force [N]
    std::vector<double> integral_mx; ///< cumulative flapwise moment [Nm]
    std::vector<double> integral_my; ///< cumulative edgewise moment [Nm]
    std::vector<double> integral_mz; ///< cumulative torsion moment  [Nm]
};

// ─────────────────────────────────────────────────────────────────────────────

class BEMPostprocessor final : public IBEMPostprocessor
{
public:
    /**
     * @param turbine        Blade geometry (chord, radius, twist, aero centres).
     * @param sim_config     Physics constants (air_density).
     * @param flow_calc      Local velocity field per section.
     * @param num_blades     Number of rotor blades.
     */
    BEMPostprocessor(TurbineGeometry const *turbine,
                     ISimulationConfig const *sim_config,
                     FlowCalculator const *flow_calc,
                     double num_blades);

    // IBEMPostprocessor
    void Process(IBEMSolver const &solver) override;
    bool Success() const override { return success_; }

    /// Access result after a successful Process() call.
    BEMPostprocessResult const &Result() const { return result_; }

private:
    // ── Injected dependencies (non-owning) ───────────────────────────────────
    TurbineGeometry const *turbine_;
    ISimulationConfig const *sim_config_;
    FlowCalculator const *flow_calc_;
    double num_blades_;

    // ── Result storage ────────────────────────────────────────────────────────
    BEMPostprocessResult result_;
    bool success_{false};
    std::size_t n_sec_{0};

    // ── Processing steps (mirror of legacy PostProcess methods) ──────────────
    void AllocateArrays();
    void ComputeElementLengths(TurbineGeometry const &tg);
    void ComputeLocalFlowAngles(IBEMSolver const &solver);
    void ComputeLocalElementLoads(IBEMSolver const &solver);
    void ComputePowerAndThrust(IBEMSolver const &solver);
    void ComputeFullBladeMoments();
    void ComputeIntegratedLoads();
};
