#pragma once
/**
 * @file ConfigurationAdapter.h
 * @brief Adapter that bridges Configuration → ISimulationConfig.
 *
 * ── What this does ───────────────────────────────────────────────────────────
 *
 *  Configuration stores everything as key/value strings.
 *  ISimulationConfig is the typed interface the solver and controller need.
 *  This adapter translates between them.
 *
 *  Zero changes to Configuration are required.  The adapter holds a const
 *  reference to it and forwards each virtual call to getDouble(key).
 *
 * ── Keys already in your main.cpp schema ────────────────────────────────────
 *
 *   "rated_electrical_power"  →  rated_power()
 *   "rated_rotorspeed"        →  speed_setpoint_rpm()  (n_soll)
 *   "min_rotorspeed"          →  min_speed_rpm()
 *   "windspeed_start"         →  wind_speed_start()
 *   "windspeed_end"           →  wind_speed_end()
 *   "windspeed_step"          →  wind_speed_step()
 *
 * ── Keys you must ADD to your schema in main.cpp ────────────────────────────
 *
 *   schema.addDouble("n_nenn", ...)          →  max_speed_rpm()
 *   schema.addDouble("lambda_opt", ...)      →  optimal_tsr()
 *   schema.addDouble("max_dp_dn", ...)       →  max_power_speed_gradient()
 *   schema.addString("power_mode", ...)      →  power_mode()   ("L0" or "POWER")
 *   schema.addDouble("kinematic_viscosity",..)→  kinematic_viscosity()   e.g. 1.5e-5
 *   schema.addDouble("speed_of_sound", ...)  →  speed_of_sound()         e.g. 340.0
 *   schema.addDouble("air_density", ...)     →  air_density()            e.g. 1.225
 *   schema.addDouble("convergence_tol", ...) →  convergence_tolerance()  e.g. 1e-6
 *   schema.addDouble("wake_transition",..)   →  wake_transition_point()  e.g. 0.4
 *   schema.addDouble("tip_extra_dist", ...)  →  tip_extra_distance()     e.g. 0.01
 *   schema.addDouble("aep_k_factor", ...)    →  weibull_k()
 *   schema.addDouble("aep_money_kwh", ...)   →  energy_price_per_kwh()
 *   schema.addRange("aep_vmean_range","vmean_start","vmean_end","vmean_step",...)
 */
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include "ISimulationConfig.h"
#include "Configuration.h" // your existing Configuration class

class ConfigurationAdapter final : public ISimulationConfig
{
public:
    explicit ConfigurationAdapter(Configuration const &cfg)
        : cfg_(cfg) {}

    // ── Physics constants ─────────────────────────────────────────────────────
    double kinematic_viscosity() const override
    {
        return cfg_.getDouble("kinematic_viscosity");
    }
    double speed_of_sound() const override
    {
        return cfg_.getDouble("speed_of_sound");
    }
    double air_density() const override
    {
        return cfg_.getDouble("air_density");
    }

    // ── BEM solver numerics ───────────────────────────────────────────────────
    double convergence_tolerance() const override
    {
        return cfg_.getDouble("convergence_tol");
    }
    double wake_transition_point() const override
    {
        return cfg_.getDouble("wake_transition");
    }
    double tip_extra_distance() const override
    {
        return cfg_.getDouble("tip_extra_dist");
    }

    // ── Control parameters ────────────────────────────────────────────────────
    double rated_power() const override
    {
        return cfg_.getDouble("rated_electrical_power"); // already in schema
    }
    double speed_setpoint_rpm() const override
    {
        // "rated_rotorspeed" = rotor speed at rated conditions = n_soll
        return cfg_.getDouble("rated_rotorspeed"); // already in schema
    }
    double max_speed_rpm() const override
    {
        return cfg_.getDouble("n_nenn"); // add to schema
    }
    double min_speed_rpm() const override
    {
        return cfg_.getDouble("min_rotorspeed"); // already in schema
    }
    double optimal_tsr() const override
    {
        return cfg_.getDouble("lambda_opt"); // add to schema
    }
    double max_power_speed_gradient() const override
    {
        return cfg_.getDouble("max_dp_dn"); // add to schema
    }
    std::string power_mode() const override
    {
        return cfg_.getString("power_mode"); // add to schema
    }

    // ── Wind speed sweep ──────────────────────────────────────────────────────
    double wind_speed_start() const override
    {
        return cfg_.getDouble("windspeed_start"); // already in schema
    }
    double wind_speed_end() const override
    {
        return cfg_.getDouble("windspeed_end"); // already in schema
    }
    double wind_speed_step() const override
    {
        return cfg_.getDouble("windspeed_step"); // already in schema
    }

    // ── AEP parameters ────────────────────────────────────────────────────────
    double weibull_k() const override
    {
        return cfg_.getDouble("aep_k_factor"); // add to schema
    }
    double energy_price_per_kwh() const override
    {
        return cfg_.getDouble("aep_money_kwh"); // add to schema
    }
    std::vector<double> mean_wind_speeds() const override
    {
        // Build vmean vector from range keys (same pattern as vinf vector)
        std::vector<double> v;
        double start = cfg_.getDouble("vmean_start");
        double end = cfg_.getDouble("vmean_end");
        double step = cfg_.getDouble("vmean_step");
        for (double x = start; x <= end + step * 1e-9; x += step)
            v.push_back(x);
        return v;
    }
    double wind_speed_bin_width() const override
    {
        // Half the wind speed step, matching original opermot dv calculation
        return cfg_.getDouble("windspeed_step") / 2.0;
    }

private:
    Configuration const &cfg_;
};
