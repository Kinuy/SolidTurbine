#include "bladenoise/noise/TurbulentInflowNoiseSource.h"
#include "bladenoise/noise/Directivity.h"
#include "bladenoise/math/SpecialFunctions.h"
#include "bladenoise/core/Constants.h"
#include <cmath>

namespace bladenoise
{
    namespace noise
    {

        using namespace constants;
        using math::MathUtils;

        TurbulentInflowNoiseSource::TurbulentInflowNoiseSource(TINoiseMethod method)
            : method_(method) {}

        Real TurbulentInflowNoiseSource::von_karman_spectrum(
            Real /*k1*/, Real /*length_scale*/, Real /*turbulence_intensity*/, Real /*velocity*/) const
        {
            return 0.0;
        }

        Real TurbulentInflowNoiseSource::amiet_response(Real /*k_chord*/, Real /*mach*/) const
        {
            return 0.0;
        }

        Real TurbulentInflowNoiseSource::thickness_correction(
            Real freq, Real chord, Real velocity, Real t_1p, Real t_10p) const
        {
            // Guidati thickness correction
            // Only applied for GUIDATI and SIMPLIFIED methods, NOT for AMIET
            // This accounts for the leading edge thickness effect on turbulence
            // distortion (the Guidati extension to BPM/Amiet)

            Real strouhal = freq * chord / velocity;
            Real t_avg = (t_1p + t_10p) / 2.0;

            if (strouhal < 1.0 || t_avg < 1e-6)
            {
                return 0.0;
            }
            else
            {
                Real corr = -20.0 * t_avg * std::log10(strouhal);
                return std::max(corr, -30.0);
            }
        }

        bool TurbulentInflowNoiseSource::calculate(
            const ProjectConfig &config,
            const BoundaryLayerState & /*upper_bl*/,
            const BoundaryLayerState & /*lower_bl*/,
            const RealVector &frequencies,
            NoiseResult &result)
        {
            const size_t num_freq = frequencies.size();
            result.spl.resize(num_freq, 0.0);

            // Extract configuration parameters (matching Fortran variable names)
            Real U = config.freestream_velocity;
            Real C0 = config.speed_of_sound;
            Real Mach = U / C0;
            Real Chord = config.chord;
            Real d = config.span;
            Real RObs = config.observer_distance;
            Real AirDens = config.air_density;
            Real LTurb = config.turbulence_length_scale;
            Real TINoise = config.turbulence_intensity / 100.0; // Convert percentage
            Real ALPSTAR = config.angle_of_attack;
            Real THETA = config.observer_theta;
            Real PHI = config.observer_phi;

            // Check for valid turbulence intensity
            if (TINoise <= 0.0)
            {
                std::fill(result.spl.begin(), result.spl.end(), 0.0);
                result.overall_spl = 0.0;
                return true;
            }

            // Calculate directivity factors
            Real DBARL = Directivity::low_frequency(Mach, THETA, PHI);
            Real DBARH = Directivity::high_frequency(Mach, THETA, PHI);

            // Cutoff frequency between high & low directivity
            Real Frequency_cutoff = 10.0 * U / (PI * Chord);

            // Wavenumber of energy-containing eddies
            Real Ke = 3.0 / (4.0 * LTurb);

            // Prandtl-Glauert correction
            Real Beta2 = 1.0 - Mach * Mach;

            // ======================================================================
            // IMPORTANT: The original Fortran NAFNoise uses ALPSTAR in DEGREES
            // in the formula: 10.*ALOG10(1 + 9.0*ALPSTAR*ALPSTAR)
            // where ALPSTAR is the angle of attack in degrees.
            //
            // The previous C++ code was INCORRECTLY converting to radians first.
            // This causes a significant reduction in the AoA correction
            // (e.g., at 6 deg: 10*log10(1+9*6^2) = 25.1 dB vs
            //                   10*log10(1+9*0.1047^2) = 0.4 dB)
            //
            // After reviewing the Fortran source, the correct interpretation is
            // that the Fortran code uses degrees directly in this formula.
            // ======================================================================

            for (size_t i = 0; i < num_freq; ++i)
            {
                Real freq = frequencies[i];

                // Select directivity based on frequency
                Real Directivity_val;
                if (freq <= Frequency_cutoff)
                {
                    Directivity_val = DBARL;
                }
                else
                {
                    Directivity_val = DBARH;
                }

                // Wave number
                Real WaveNumber = TWO_PI * freq / U;

                // Normalized wavenumbers
                Real Kbar = WaveNumber * Chord / 2.0;
                Real Khat = WaveNumber / Ke;

                // High-frequency SPL calculation (main Amiet formula from Fortran)
                Real term1 = AirDens * AirDens * std::pow(C0, 4.0) * LTurb * (d / 2.0);
                Real term2 = RObs * RObs;
                Real term3 = std::pow(Mach, 5.0) * TINoise * TINoise;
                Real term4 = std::pow(Khat, 3.0) * std::pow(1.0 + Khat * Khat, -7.0 / 3.0);

                Real SPLhigh = 10.0 * std::log10(term1 / term2 * term3 * term4 *
                               Directivity_val) + 78.4;

                // Angle of attack correction - uses DEGREES directly as in Fortran
                SPLhigh = SPLhigh + 10.0 * std::log10(1.0 + 9.0 * ALPSTAR * ALPSTAR);

                // Sears function approximation
                Real Sears = 1.0 / (TWO_PI * Kbar / Beta2 +
                             1.0 / (1.0 + 2.4 * Kbar / Beta2));

                // Low-frequency correction factor
                Real LFC = 10.0 * Sears * Mach * Kbar * Kbar / Beta2;

                // Final SPL with low-frequency correction
                Real spl = SPLhigh + 10.0 * std::log10(LFC / (1.0 + LFC));

                // Apply thickness correction ONLY for Guidati methods
                // The original Amiet/BPM method does NOT include thickness correction
                if (method_ == TINoiseMethod::GUIDATI ||
                    method_ == TINoiseMethod::SIMPLIFIED)
                {
                    spl += thickness_correction(freq, Chord, U,
                                                config.thickness_1_percent,
                                                config.thickness_10_percent);
                }

                result.spl[i] = spl;
            }

            result.overall_spl = MathUtils::compute_OASPL(result.spl);

            return true;
        }

    } // namespace noise
} // namespace bladenoise
