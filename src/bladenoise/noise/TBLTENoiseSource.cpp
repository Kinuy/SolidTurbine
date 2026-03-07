#include "bladenoise/noise/TBLTENoiseSource.h"
#include "bladenoise/noise/Directivity.h"
#include "bladenoise/math/SpecialFunctions.h"
#include "bladenoise/core/Constants.h"
#include <cmath>
#include <algorithm>

namespace bladenoise
{
    namespace noise
    {

        using namespace constants;
        using math::MathUtils;

        //==============================================================================
        // A_min: Minimum A-curve (corresponds to Fortran AMIN subroutine)
        //==============================================================================
        Real TBLTENoiseSource::A_min(Real a) const
        {
            Real x1 = std::abs(a);

            if (x1 <= 0.204)
            {
                return std::sqrt(67.552 - 886.788 * x1 * x1) - 8.219;
            }
            else if (x1 <= 0.244)
            {
                return -32.665 * x1 + 3.981;
            }
            else
            {
                return -142.795 * x1 * x1 * x1 + 103.656 * x1 * x1 - 57.757 * x1 + 6.006;
            }
        }

        //==============================================================================
        // A_max: Maximum A-curve (corresponds to Fortran AMAX subroutine)
        //==============================================================================
        Real TBLTENoiseSource::A_max(Real a) const
        {
            Real x1 = std::abs(a);

            if (x1 <= 0.13)
            {
                return std::sqrt(67.552 - 886.788 * x1 * x1) - 8.219;
            }
            else if (x1 <= 0.321)
            {
                return -15.901 * x1 + 1.098;
            }
            else
            {
                return -4.669 * x1 * x1 * x1 + 3.491 * x1 * x1 - 16.699 * x1 + 1.149;
            }
        }

        //==============================================================================
        // B_min: Minimum B-curve (corresponds to Fortran BMIN subroutine)
        //==============================================================================
        Real TBLTENoiseSource::B_min(Real b) const
        {
            Real x1 = std::abs(b);

            if (x1 <= 0.13)
            {
                return std::sqrt(16.888 - 886.788 * x1 * x1) - 4.109;
            }
            else if (x1 <= 0.145)
            {
                return -83.607 * x1 + 8.138;
            }
            else
            {
                return -817.81 * x1 * x1 * x1 + 355.21 * x1 * x1 - 135.024 * x1 + 10.619;
            }
        }

        //==============================================================================
        // B_max: Maximum B-curve (corresponds to Fortran BMAX subroutine)
        //==============================================================================
        Real TBLTENoiseSource::B_max(Real b) const
        {
            Real x1 = std::abs(b);

            if (x1 <= 0.1)
            {
                return std::sqrt(16.888 - 886.788 * x1 * x1) - 4.109;
            }
            else if (x1 <= 0.187)
            {
                return -31.313 * x1 + 1.854;
            }
            else
            {
                return -80.541 * x1 * x1 * x1 + 44.174 * x1 * x1 - 39.381 * x1 + 2.344;
            }
        }

        //==============================================================================
        // compute_A0: Where A-curve takes value of -20 dB (Fortran A0COMP)
        //==============================================================================
        Real TBLTENoiseSource::compute_A0(Real rc) const
        {
            if (rc < 9.52e4)
            {
                return 0.57;
            }
            else if (rc < 8.57e5)
            {
                return (-9.57e-13) * std::pow(rc - 8.57e5, 2.0) + 1.13;
            }
            else
            {
                return 1.13;
            }
        }

        //==============================================================================
        // compute_B0: For B-curve calculations
        //==============================================================================
        Real TBLTENoiseSource::compute_B0(Real rc) const
        {
            if (rc < 9.52e4)
            {
                return 0.30;
            }
            else if (rc < 8.57e5)
            {
                return (-4.48e-13) * std::pow(rc - 8.57e5, 2.0) + 0.56;
            }
            else
            {
                return 0.56;
            }
        }

        //==============================================================================
        // compute_K1: Amplitude function K1
        //==============================================================================
        Real TBLTENoiseSource::compute_K1(Real rc) const
        {
            if (rc < 2.47e5)
            {
                return -4.31 * std::log10(rc) + 156.3;
            }
            else if (rc < 8.0e5)
            {
                return -9.0 * std::log10(rc) + 181.6;
            }
            else
            {
                return 128.5;
            }
        }

        //==============================================================================
        // compute_K2: Amplitude function K2
        //==============================================================================
        Real TBLTENoiseSource::compute_K2(Real K1, Real /*rc*/, Real mach, Real alpha) const
        {
            Real gamma = 27.094 * mach + 3.31;
            Real beta = 72.650 * mach + 10.74;
            Real gamma0 = 23.430 * mach + 4.651;
            Real beta0 = -34.190 * mach - 13.820;

            Real K2;
            if (alpha <= (gamma0 - gamma))
            {
                K2 = -1000.0;
            }
            else if (alpha <= (gamma0 + gamma))
            {
                K2 = std::sqrt(beta * beta - std::pow(beta / gamma, 2.0) *
                                                 std::pow(alpha - gamma0, 2.0)) +
                     beta0;
            }
            else
            {
                K2 = -12.0;
            }

            return K2 + K1;
        }

        //==============================================================================
        // compute_delta_K1: Correction to K1
        //==============================================================================
        Real TBLTENoiseSource::compute_delta_K1(Real Re_dstar_p, Real alpha) const
        {
            if (Re_dstar_p <= 5000.0)
            {
                return -alpha * (5.29 - 1.43 * std::log10(Re_dstar_p));
            }
            else
            {
                return 0.0;
            }
        }

        //==============================================================================
        // G1_function: Spectral shape for laminar vortex shedding noise
        // (Fortran G1COMP / LBLVS subroutine)
        //==============================================================================
        Real TBLTENoiseSource::G1_function(Real e) const
        {
            // G1 function from BPM model for LBL-VS noise spectral shape
            // e = St'/St'_peak ratio parameter

            Real e_abs = std::abs(e);

            if (e_abs <= 0.5974)
            {
                return 39.8 * std::log10(e_abs) - 11.12;
            }
            else if (e_abs <= 0.8545)
            {
                return 98.409 * std::log10(e_abs) + 2.0;
            }
            else if (e_abs <= 1.17)
            {
                return -5.076 + std::sqrt(2.484 - 506.25 *
                                                      std::pow(std::log10(e_abs), 2.0));
            }
            else if (e_abs <= 1.674)
            {
                return -98.409 * std::log10(e_abs) + 2.0;
            }
            else
            {
                return -39.8 * std::log10(e_abs) - 11.12;
            }
        }

        //==============================================================================
        // G2_function: Reynolds number dependent function for LBL-VS
        //==============================================================================
        Real TBLTENoiseSource::G2_function(Real Re_dstar) const
        {
            // G2 function controls LBL-VS amplitude based on Reynolds number
            Real log_re = std::log10(Re_dstar);

            if (Re_dstar <= 1.3e5)
            {
                return 77.852 * log_re - 114.21;
            }
            else if (Re_dstar <= 4.0e5)
            {
                return 65.188 * log_re - 60.716;
            }
            else
            {
                return -114.052 * log_re * log_re + 705.536 * log_re - 1010.01;
            }
        }

        //==============================================================================
        // G3_function: Angle of attack function for LBL-VS
        //==============================================================================
        Real TBLTENoiseSource::G3_function(Real alpha) const
        {
            // G3 function: angle of attack correction for LBL-VS noise
            return 171.04 - 3.03 * alpha;
        }

        //==============================================================================
        // compute_laminar_vortex_shedding: LBL-VS noise (Fortran LBLVS)
        //==============================================================================
        void TBLTENoiseSource::compute_laminar_vortex_shedding(
            const ProjectConfig &config,
            const BoundaryLayerState &upper_bl,
            const BoundaryLayerState &lower_bl,
            const RealVector &frequencies,
            NoiseResult &lam_result)
        {
            const size_t num_freq = frequencies.size();
            lam_result.spl.resize(num_freq, -100.0);

            Real mach = config.mach_number();
            Real velocity = config.freestream_velocity;
            Real visc = config.kinematic_viscosity;
            Real span = config.span;
            Real distance = config.observer_distance;
            Real alpha = std::abs(config.angle_of_attack);

            // Use pressure-side displacement thickness for LBL-VS
            Real dstrp = lower_bl.displacement_thickness;

            // Reynolds number based on pressure side displacement thickness
            Real Re_dstrp = dstrp * velocity / visc;

            // LBL-VS only applies when the pressure side BL is LAMINAR.
            // Skip if:
            //  - Pressure side BL is turbulent (forced trip or natural transition)
            //  - Re_delta* is outside the valid range for the G2 correlation
            if (lower_bl.is_turbulent || Re_dstrp > 1.6e5 || Re_dstrp < 1.0e4)
            {
                lam_result.overall_spl = 0; //-100.0;
                return;
            }

            // High-frequency directivity
            Real Dbarh = Directivity::high_frequency(mach, config.observer_theta,
                                                     config.observer_phi);

            // Peak Strouhal number for LBL-VS
            // St'_peak = 0.1 * Re_dstar_p^(-0.55) (BPM Eq. 54)
            Real St_peak;
            if (Re_dstrp <= 1.3e5)
            {
                St_peak = 0.18;
            }
            else if (Re_dstrp <= 4.0e5)
            {
                St_peak = 0.001756 * std::pow(Re_dstrp, 0.3931);
            }
            else
            {
                St_peak = 0.28;
            }

            // G2 amplitude function
            Real G2 = G2_function(Re_dstrp);

            // G3 angle of attack function
            Real G3 = G3_function(alpha);

            // Scaling: 10*log10(delta*_p * M^5 * Dbarh * L / r^2)
            Real scale = 10.0 * std::log10(
                                    dstrp * std::pow(mach, 5.0) * Dbarh * span /
                                    (distance * distance));

            for (size_t i = 0; i < num_freq; ++i)
            {
                Real freq = frequencies[i];

                // Strouhal number
                Real Stp = freq * dstrp / velocity;

                // Ratio to peak Strouhal
                Real e = Stp / St_peak;

                // Spectral shape G1
                Real G1 = G1_function(e);

                // LBL-VS SPL
                Real spl_lam = G1 + G2 + G3 + scale;

                lam_result.spl[i] = spl_lam;
            }

            lam_result.overall_spl = MathUtils::compute_OASPL(lam_result.spl);
        }

        //==============================================================================
        // Main calculation function - follows Fortran TBLTE subroutine structure
        //==============================================================================
        bool TBLTENoiseSource::calculate(
            const ProjectConfig &config,
            const BoundaryLayerState &upper_bl,
            const BoundaryLayerState &lower_bl,
            const RealVector &frequencies,
            NoiseResult &result)
        {
            const size_t num_freq = frequencies.size();
            result.spl.resize(num_freq, 0.0);
            pressure_side_result.spl.resize(num_freq, 0.0);
            suction_side_result.spl.resize(num_freq, 0.0);
            separation_result.spl.resize(num_freq, 0.0);
            laminar_result.spl.resize(num_freq, 0.0); //-100.0);

            // Flow parameters
            Real mach = config.mach_number();
            Real rc = config.reynolds_number();
            Real alpha = config.angle_of_attack;
            Real velocity = config.freestream_velocity;
            Real visc = config.kinematic_viscosity;
            Real span = config.span;
            Real distance = config.observer_distance;

            // Boundary layer displacement thicknesses
            Real dstrp = lower_bl.displacement_thickness; // Pressure side
            Real dstrs = upper_bl.displacement_thickness; // Suction side

            // Reynolds numbers based on displacement thickness
            Real Re_dstrs = dstrs * velocity / visc;
            Real Re_dstrp = dstrp * velocity / visc;

            // Compute directivity functions
            Real Dbarl = Directivity::low_frequency(mach, config.observer_theta,
                                                    config.observer_phi);
            Real Dbarh = Directivity::high_frequency(mach, config.observer_theta,
                                                     config.observer_phi);

            // Determine peak Strouhal numbers for A and B curves
            Real St1 = 0.02 * std::pow(mach, -0.6);

            Real St2;
            if (alpha <= 1.333)
            {
                St2 = St1;
            }
            else if (alpha <= 12.5)
            {
                St2 = St1 * std::pow(10.0, 0.0054 * std::pow(alpha - 1.333, 2.0));
            }
            else
            {
                St2 = 4.72 * St1;
            }

            Real St1_prime = (St1 + St2) / 2.0;

            // Compute A0 and evaluate min/max A-curves at A0
            Real A0 = compute_A0(rc);
            Real A02 = compute_A0(3.0 * rc);

            Real Amin_A0 = A_min(A0);
            Real Amax_A0 = A_max(A0);
            Real Amin_A02 = A_min(A02);
            Real Amax_A02 = A_max(A02);

            // Compute A max/min ratio (interpolation factor)
            Real Ar_A0 = (Amin_A0 != Amax_A0) ? (20.0 + Amin_A0) / (Amin_A0 - Amax_A0) : 0.0;
            Real Ar_A02 = (Amin_A02 != Amax_A02) ? (20.0 + Amin_A02) / (Amin_A02 - Amax_A02) : 0.0;

            // Compute B0 and evaluate min/max B-curves at B0
            Real B0 = compute_B0(rc);
            Real Bmin_B0 = B_min(B0);
            Real Bmax_B0 = B_max(B0);

            // Compute B max/min ratio
            Real Br_B0 = (Bmin_B0 != Bmax_B0) ? (20.0 + Bmin_B0) / (Bmin_B0 - Bmax_B0) : 0.0;

            // Amplitude functions
            Real K1 = compute_K1(rc);
            Real K2 = compute_K2(K1, rc, mach, alpha);

            // Check for angle of attack contribution
            Real gamma0 = 23.430 * mach + 4.651;
            Real xcheck = gamma0;

            // For each frequency
            for (size_t i = 0; i < num_freq; ++i)
            {
                Real freq = frequencies[i];

                // Strouhal numbers
                Real Stp = freq * dstrp / velocity;
                Real Sts = freq * dstrs / velocity;

                // =============================================
                // Pressure side A-computation
                // =============================================
                Real a_p = std::log10(Stp / St1);
                Real Amin_a = A_min(a_p);
                Real Amax_a = A_max(a_p);
                Real AA_p = Amin_a + Ar_A0 * (Amax_a - Amin_a);

                // Delta K1 correction
                Real delK1 = compute_delta_K1(Re_dstrp, alpha);

                // Pressure side SPL
                Real spl_p = AA_p + K1 - 3.0 +
                             10.0 * std::log10(dstrp * std::pow(mach, 5.0) * Dbarh * span /
                                               (distance * distance)) +
                             delK1;

                // =============================================
                // Suction side computation
                // =============================================
                bool use_switch = (alpha >= xcheck) || (alpha > 12.5);

                Real spl_s, spl_alpha;

                if (!use_switch)
                {
                    // Normal A-computation for suction side
                    Real a_s = std::log10(Sts / St1_prime);
                    Real Amin_as = A_min(a_s);
                    Real Amax_as = A_max(a_s);
                    Real AA_s = Amin_as + Ar_A0 * (Amax_as - Amin_as);

                    // Delta K1 correction for suction side
                    Real delK1_s = compute_delta_K1(Re_dstrs, alpha);

                    spl_s = AA_s + K1 - 3.0 +
                            10.0 * std::log10(dstrs * std::pow(mach, 5.0) * Dbarh * span /
                                              (distance * distance)) +
                            delK1_s;

                    // B-curve computation for separation/alpha contribution
                    Real b = std::abs(std::log10(Sts / St2));
                    Real Bmin_b = B_min(b);
                    Real Bmax_b = B_max(b);
                    Real BB = Bmin_b + Br_B0 * (Bmax_b - Bmin_b);

                    spl_alpha = BB + K2 +
                                10.0 * std::log10(dstrs * std::pow(mach, 5.0) * Dbarh * span /
                                                  (distance * distance));
                }
                else
                {
                    // SWITCH is TRUE
                    spl_s = 0.0 + 10.0 * std::log10(dstrs * std::pow(mach, 5.0) * Dbarl * span /
                                                    (distance * distance));
                    spl_p = 0.0 + 10.0 * std::log10(dstrs * std::pow(mach, 5.0) * Dbarl * span /
                                                    (distance * distance));

                    Real b = std::abs(std::log10(Sts / St2));
                    Real Amin_b = A_min(b);
                    Real Amax_b = A_max(b);
                    Real BB = Amin_b + Ar_A02 * (Amax_b - Amin_b);

                    spl_alpha = BB + K2 +
                                10.0 * std::log10(dstrs * std::pow(mach, 5.0) * Dbarl * span /
                                                  (distance * distance));
                }

                // Clamp minimum values
                spl_p = std::max(spl_p, -100.0);
                spl_s = std::max(spl_s, -100.0);
                spl_alpha = std::max(spl_alpha, -100.0);

                // Store individual contributions
                pressure_side_result.spl[i] = spl_p;
                suction_side_result.spl[i] = spl_s;
                separation_result.spl[i] = spl_alpha;

                // Sum contributions on mean-square pressure basis
                Real P1 = std::pow(10.0, spl_p / 10.0);
                Real P2 = std::pow(10.0, spl_s / 10.0);
                Real P4 = std::pow(10.0, spl_alpha / 10.0);

                result.spl[i] = 10.0 * std::log10(P1 + P2 + P4);
            }

            // =============================================
            // Laminar Boundary Layer Vortex Shedding (LBL-VS)
            // This was MISSING from the original C++ implementation
            // =============================================
            if (config.compute_laminar)
            {
                compute_laminar_vortex_shedding(config, upper_bl, lower_bl,
                                                frequencies, laminar_result);

                // Add LBL-VS to total TBL-TE result
                for (size_t i = 0; i < num_freq; ++i)
                {
                    if (laminar_result.spl[i] > -100.0)
                    {
                        Real P_total = std::pow(10.0, result.spl[i] / 10.0);
                        Real P_lam = std::pow(10.0, laminar_result.spl[i] / 10.0);
                        result.spl[i] = 10.0 * std::log10(P_total + P_lam);
                    }
                }
            }

            // Compute overall SPL
            result.overall_spl = MathUtils::compute_OASPL(result.spl);
            pressure_side_result.overall_spl = MathUtils::compute_OASPL(pressure_side_result.spl);
            suction_side_result.overall_spl = MathUtils::compute_OASPL(suction_side_result.spl);
            separation_result.overall_spl = MathUtils::compute_OASPL(separation_result.spl);
            laminar_result.overall_spl = MathUtils::compute_OASPL(laminar_result.spl);

            return true;
        }

    } // namespace noise
} // namespace bladenoise
