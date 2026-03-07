#include "bladenoise/airfoil/BPMBoundaryLayerCalculator.h"
#include "bladenoise/airfoil/XfoilBoundaryLayerCalculator.h"
#include "bladenoise/core/Constants.h"
#include <cmath>

namespace bladenoise
{
    namespace airfoil
    {

        void BPMBoundaryLayerCalculator::calculate_thicknesses(
            Real chord, Real velocity, Real alpha, TripConfig trip,
            Real &delta_p, Real &delta_s, Real &delta_p_star,
            Real c0, Real visc)
        {
            // Calculate Mach number and Reynolds number
            //Real mach = velocity / c0;
            Real reynolds = velocity * chord / visc;

            // Absolute angle of attack
            Real alpha_abs = std::abs(alpha);

            // =================================================================
            // Calculate boundary layer and displacement thicknesses using
            // BPM correlations from NASA RP-1218 (Brooks, Pope, Marcolini 1989)
            //
            // The original Fortran NAFNoise code computes BOTH boundary layer
            // thickness (delta) AND displacement thickness (delta*) from
            // separate correlation equations. Using a fixed ratio (delta*/delta)
            // is incorrect and leads to significant errors.
            // =================================================================

            Real log_re = std::log10(reynolds);

            if (trip == TripConfig::NO_TRIP)
            {
                // ==========================================================
                // Untripped boundary layer
                // ==========================================================

                // --- Boundary layer thickness (delta) ---
                // Pressure side BL thickness at zero AoA
                Real delta0 = std::pow(10.0,
                    1.6569 - 0.9045 * log_re + 0.0596 * log_re * log_re);

                // Pressure side delta
                delta_p = delta0 * chord * std::pow(10.0,
                    -0.04175 * alpha + 0.00106 * alpha * alpha);

                // Suction side delta
                if (alpha_abs <= 5.0)
                {
                    delta_s = delta0 * chord * std::pow(10.0, 0.0311 * alpha);
                }
                else if (alpha_abs <= 12.5)
                {
                    delta_s = 0.381 * chord * std::pow(reynolds, -0.2) *
                              std::pow(10.0, 0.1231 * alpha);
                }
                else
                {
                    delta_s = 14.296 * chord * std::pow(reynolds, -0.2) *
                              std::pow(10.0, 0.0258 * alpha);
                }

                // --- Displacement thickness (delta*) ---
                // BPM Eqs. for displacement thickness - pressure side
                // delta*_p at zero AoA
                Real dstar0_p = std::pow(10.0,
                    3.0187 - 1.5397 * log_re + 0.1059 * log_re * log_re);

                delta_p_star = dstar0_p * chord * std::pow(10.0,
                    -0.0432 * alpha + 0.00113 * alpha * alpha);

                // delta*_s (suction side displacement thickness) stored externally
                // We need to also compute it here for the suction side
            }
            else
            {
                // ==========================================================
                // Tripped boundary layer (light or heavy trip)
                // ==========================================================

                // --- Boundary layer thickness (delta) ---
                Real delta0 = std::pow(10.0,
                    1.892 - 0.9045 * log_re + 0.0596 * log_re * log_re);
                delta_p = delta0 * chord;

                if (trip == TripConfig::HEAVY_TRIP)
                {
                    delta_p *= std::pow(10.0, -0.0601 * alpha);
                }
                else
                {
                    // Light trip
                    delta_p *= std::pow(10.0,
                        -0.04175 * alpha + 0.00106 * alpha * alpha);
                }

                // Suction side delta
                if (trip == TripConfig::HEAVY_TRIP)
                {
                    if (alpha_abs <= 5.0)
                    {
                        delta_s = delta0 * chord * std::pow(10.0, 0.0679 * alpha);
                    }
                    else if (alpha_abs <= 12.5)
                    {
                        delta_s = 0.0162 * chord * std::pow(10.0, 0.3066 * alpha);
                    }
                    else
                    {
                        delta_s = 52.42 * chord * std::pow(reynolds, -0.2) *
                                  std::pow(10.0, 0.0258 * alpha);
                    }
                }
                else
                {
                    // Light trip
                    if (alpha_abs <= 7.5)
                    {
                        delta_s = delta0 * chord * std::pow(10.0, 0.0679 * alpha);
                    }
                    else if (alpha_abs <= 12.5)
                    {
                        delta_s = 0.381 * chord * std::pow(reynolds, -0.2) *
                                  std::pow(10.0, 0.1516 * alpha);
                    }
                    else
                    {
                        delta_s = 14.296 * chord * std::pow(reynolds, -0.2) *
                                  std::pow(10.0, 0.0258 * alpha);
                    }
                }

                // --- Displacement thickness (delta*) ---
                // Tripped: separate BPM correlation for delta*
                Real dstar0_p = std::pow(10.0,
                    3.3721 - 1.5397 * log_re + 0.1059 * log_re * log_re);

                if (trip == TripConfig::HEAVY_TRIP)
                {
                    delta_p_star = dstar0_p * chord *
                                   std::pow(10.0, -0.0601 * alpha);
                }
                else
                {
                    // Light trip
                    delta_p_star = dstar0_p * chord *
                                   std::pow(10.0,
                                       -0.0432 * alpha + 0.00113 * alpha * alpha);
                }
            }
        }

        void BPMBoundaryLayerCalculator::calculate_displacement_thickness_suction(
            Real chord, Real velocity, Real alpha, TripConfig trip,
            Real c0, Real visc, Real &delta_s_star)
        {
            // Separate function to compute suction-side displacement thickness
            // following BPM correlations (NASA RP-1218)

            Real reynolds = velocity * chord / visc;
            Real alpha_abs = std::abs(alpha);
            Real log_re = std::log10(reynolds);

            if (trip == TripConfig::NO_TRIP)
            {
                // Untripped suction side displacement thickness
                Real dstar0_s = std::pow(10.0,
                    3.0187 - 1.5397 * log_re + 0.1059 * log_re * log_re);

                if (alpha_abs <= 5.0)
                {
                    delta_s_star = dstar0_s * chord *
                                   std::pow(10.0, 0.0679 * alpha);
                }
                else if (alpha_abs <= 12.5)
                {
                    delta_s_star = dstar0_s * chord * 0.0162 *
                                   std::pow(10.0, 0.3066 * alpha);
                }
                else
                {
                    delta_s_star = dstar0_s * chord * 52.42 *
                                   std::pow(reynolds, -0.2) *
                                   std::pow(10.0, 0.0258 * alpha);
                }
            }
            else
            {
                // Tripped suction side displacement thickness
                Real dstar0_s = std::pow(10.0,
                    3.3721 - 1.5397 * log_re + 0.1059 * log_re * log_re);

                if (trip == TripConfig::HEAVY_TRIP)
                {
                    if (alpha_abs <= 5.0)
                    {
                        delta_s_star = dstar0_s * chord *
                                       std::pow(10.0, 0.0679 * alpha);
                    }
                    else if (alpha_abs <= 12.5)
                    {
                        delta_s_star = dstar0_s * chord * 0.0162 *
                                       std::pow(10.0, 0.3066 * alpha);
                    }
                    else
                    {
                        delta_s_star = dstar0_s * chord * 52.42 *
                                       std::pow(reynolds, -0.2) *
                                       std::pow(10.0, 0.0258 * alpha);
                    }
                }
                else
                {
                    // Light trip
                    if (alpha_abs <= 7.5)
                    {
                        delta_s_star = dstar0_s * chord *
                                       std::pow(10.0, 0.0679 * alpha);
                    }
                    else if (alpha_abs <= 12.5)
                    {
                        delta_s_star = dstar0_s * chord * 0.381 *
                                       std::pow(reynolds, -0.2) *
                                       std::pow(10.0, 0.1516 * alpha);
                    }
                    else
                    {
                        delta_s_star = dstar0_s * chord * 14.296 *
                                       std::pow(reynolds, -0.2) *
                                       std::pow(10.0, 0.0258 * alpha);
                    }
                }
            }
        }

        bool BPMBoundaryLayerCalculator::calculate(
            const io::AirfoilData & /*airfoil*/,
            const ProjectConfig &config,
            BoundaryLayerState &upper_bl,
            BoundaryLayerState &lower_bl)
        {
            Real delta_p = 0.0, delta_s = 0.0, delta_p_star = 0.0;

            calculate_thicknesses(
                config.chord, config.freestream_velocity, config.angle_of_attack,
                config.trip_config, delta_p, delta_s, delta_p_star,
                config.speed_of_sound, config.kinematic_viscosity);

            // Compute suction-side displacement thickness from its own correlation
            Real delta_s_star = 0.0;
            calculate_displacement_thickness_suction(
                config.chord, config.freestream_velocity, config.angle_of_attack,
                config.trip_config, config.speed_of_sound,
                config.kinematic_viscosity, delta_s_star);

            // Assign to output structures
            // Upper (suction) side
            upper_bl.boundary_layer_thickness = delta_s;
            upper_bl.displacement_thickness = delta_s_star;
            // Momentum thickness from shape factor relationship
            // For turbulent BL, typical H ~ 1.3-1.6; use 1.4 as default
            // For laminar BL, H ~ 2.6
            Real H_turb = 1.4;
            Real H_lam = 2.59;

            bool is_upper_turb = (config.trip_config != TripConfig::NO_TRIP) ||
                                 (config.reynolds_number() > 5.0e5);

            Real H_upper = is_upper_turb ? H_turb : H_lam;
            upper_bl.momentum_thickness = upper_bl.displacement_thickness / H_upper;
            upper_bl.shape_factor = H_upper;
            upper_bl.edge_velocity = config.freestream_velocity;
            upper_bl.reynolds_theta = upper_bl.momentum_thickness *
                                      config.freestream_velocity /
                                      config.kinematic_viscosity;
            upper_bl.is_turbulent = is_upper_turb;

            // Lower (pressure) side
            lower_bl.boundary_layer_thickness = delta_p;
            lower_bl.displacement_thickness = delta_p_star;

            bool is_lower_turb = (config.trip_config != TripConfig::NO_TRIP) ||
                                 (config.reynolds_number() > 5.0e5);

            Real H_lower = is_lower_turb ? H_turb : H_lam;
            lower_bl.momentum_thickness = lower_bl.displacement_thickness / H_lower;
            lower_bl.shape_factor = H_lower;
            lower_bl.edge_velocity = config.freestream_velocity;
            lower_bl.reynolds_theta = lower_bl.momentum_thickness *
                                      config.freestream_velocity /
                                      config.kinematic_viscosity;
            lower_bl.is_turbulent = is_lower_turb;

            return true;
        }

    } // namespace airfoil
} // namespace bladenoise
