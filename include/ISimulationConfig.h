#pragma once
/**
 * @file ISimulationConfig.h
 * @brief Minimal solver-facing interface to simulation configuration.
 *
 * Interface Segregation Principle: the BEM solver, controller, and flow
 * calculator only need a small subset of what Configuration holds.
 * Expressing that subset as a pure interface means:
 *   - The solver does not depend on the full Configuration class.
 *   - Configuration implements this interface — zero extra work at call sites.
 *   - Unit tests can inject a lightweight mock.
 *
 * Dependency Inversion: SolverConfig and TurbineControllerParams are
 * populated from this interface, not from the concrete Configuration type.
 *
 * ── What maps to what ────────────────────────────────────────────────────────
 *
 *  Old InputCase method              → ISimulationConfig method
 *  ─────────────────────────────────────────────────────────────
 *  kinematic_viscosity()             → kinematic_viscosity()
 *  speed_of_sound()                  → speed_of_sound()
 *  tip_extra_distance()              → tip_extra_distance()
 *  empirical_wake_transition_point() → wake_transition_point()
 *  epsi()                            → convergence_tolerance()
 *  get_coeffmodifier()               → (handled by polars, no longer needed)
 *
 *  Old opermot_inputdata method      → ISimulationConfig method
 *  ─────────────────────────────────────────────────────────────
 *  getPmax()                         → rated_power()
 *  getNsoll()                        → speed_setpoint_rpm()
 *  getNnenn()                        → max_speed_rpm()
 *  getNmin()                         → min_speed_rpm()
 *  getLambdaOpt()                    → optimal_tsr()
 *  getMaxDpDn()                      → max_power_speed_gradient()
 *  getPowerMode()                    → power_mode()
 *  getAEPkFactor()                   → weibull_k()
 *  getAEPmoneyPerKWH()               → energy_price_per_kwh()
 *  getVmeanVector()                  → mean_wind_speeds()
 *  getVinfRangeVector()[2] / 2       → wind_speed_bin_width()
 *  air density                       → air_density()
 */
#include <string>
#include <vector>

class ISimulationConfig
{
public:
    virtual ~ISimulationConfig() = default;

    // ── Aerodynamic physics constants ─────────────────────────────────────────
    virtual double kinematic_viscosity() const = 0; ///< ν [m²/s]
    virtual double speed_of_sound() const = 0;      ///< a [m/s]
    virtual double air_density() const = 0;         ///< ρ [kg/m³]

    // ── BEM solver numerics ───────────────────────────────────────────────────
    virtual double convergence_tolerance() const = 0; ///< BEM residual tol
    virtual double wake_transition_point() const = 0; ///< Ning emp. wake x
    virtual double tip_extra_distance() const = 0;    ///< tip singularity Δ

    // ── Turbine control parameters ────────────────────────────────────────────
    virtual double rated_power() const = 0;              ///< P_max [W]
    virtual double speed_setpoint_rpm() const = 0;       ///< n_soll [rpm]
    virtual double max_speed_rpm() const = 0;            ///< n_nenn [rpm]
    virtual double min_speed_rpm() const = 0;            ///< n_min  [rpm]
    virtual double optimal_tsr() const = 0;              ///< λ_opt  [-]
    virtual double max_power_speed_gradient() const = 0; ///< dP/dn  [W/rpm]
    virtual std::string power_mode() const = 0;          ///< "L0" | "POWER"

    // ── Wind speed sweep ──────────────────────────────────────────────────────
    virtual double wind_speed_start() const = 0; ///< v_start [m/s]
    virtual double wind_speed_end() const = 0;   ///< v_end   [m/s]
    virtual double wind_speed_step() const = 0;  ///< dv      [m/s]

    // ── AEP parameters ────────────────────────────────────────────────────────
    virtual double weibull_k() const = 0;
    virtual double energy_price_per_kwh() const = 0;
    virtual std::vector<double> mean_wind_speeds() const = 0;
    virtual double wind_speed_bin_width() const = 0;
};
