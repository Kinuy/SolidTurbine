#include "bladenoise/noise/BluntnessNoiseSource.h"
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

        Real BluntnessNoiseSource::G4_function(Real hdstar, Real psi) const
        {
            // G4 function for bluntness noise amplitude
            // From Fortran BLUNT subroutine
            // HDSTAR = h / delta*_avg (bluntness over average displacement thickness)
            // PSI = trailing edge angle in degrees

            if (hdstar <= 5.0)
            {
                return 17.5 * std::log10(hdstar) + 157.5 - 1.114 * psi;
            }
            else
            {
                return 169.7 - 1.114 * psi;
            }
        }

        Real BluntnessNoiseSource::G5_function(Real hdstar, Real eta) const
        {
            // G5 function for bluntness noise spectral shape
            // From Fortran G5COMP subroutine
            // HDSTAR = h / delta*_avg
            // ETA = log10(St / St_peak) where St = f*h/U

            // Compute MU parameter
            Real mu;
            if (hdstar < 0.25)
            {
                mu = 0.1211;
            }
            else if (hdstar <= 0.62)
            {
                mu = -0.2175 * hdstar + 0.1755;
            }
            else if (hdstar < 1.15)
            {
                mu = -0.0308 * hdstar + 0.0596;
            }
            else
            {
                mu = 0.0242;
            }

            // Compute M parameter
            Real M;
            if (hdstar <= 0.02)
            {
                M = 0.0;
            }
            else if (hdstar < 0.5)
            {
                M = 68.724 * hdstar - 1.35;
            }
            else if (hdstar <= 0.62)
            {
                M = 308.475 * hdstar - 121.23;
            }
            else if (hdstar <= 1.15)
            {
                M = 224.811 * hdstar - 69.354;
            }
            else if (hdstar < 1.2)
            {
                M = 1583.28 * hdstar - 1631.592;
            }
            else
            {
                M = 268.344;
            }
            M = std::max(M, 0.0);

            // Compute ETA0 (the eta value at the peak)
            Real eta0 = -std::sqrt((M * M * mu * mu * mu * mu) /
                                   (6.25 + M * M * mu * mu));

            // Compute K parameter
            Real K = 2.5 * std::sqrt(1.0 - (eta0 / mu) * (eta0 / mu)) - 2.5 - M * eta0;

            // Compute G5 based on eta range
            Real G5;
            if (eta <= eta0)
            {
                G5 = M * eta + K;
            }
            else if (eta <= 0.0)
            {
                G5 = 2.5 * std::sqrt(1.0 - (eta / mu) * (eta / mu)) - 2.5;
            }
            else if (eta <= 0.03616)
            {
                G5 = std::sqrt(1.5625 - 1194.99 * eta * eta) - 1.25;
            }
            else
            {
                G5 = -155.543 * eta + 4.375;
            }

            return G5;
        }

        bool BluntnessNoiseSource::calculate(
            const ProjectConfig &config,
            const BoundaryLayerState &upper_bl,
            const BoundaryLayerState &lower_bl,
            const RealVector &frequencies,
            NoiseResult &result)
        {
            const size_t num_freq = frequencies.size();
            result.spl.resize(num_freq, 0.0);

            Real h = config.trailing_edge_thickness;
            Real psi = config.trailing_edge_angle;
            Real mach = config.mach_number();
            Real velocity = config.freestream_velocity;
            Real span = config.span;
            Real distance = config.observer_distance;

            // Average displacement thickness at trailing edge (DSTRAVG in Fortran)
            Real dstravg = (upper_bl.displacement_thickness +
                            lower_bl.displacement_thickness) /
                           2.0;

            // h/delta* ratio (HDSTAR in Fortran)
            Real hdstar = h / dstravg;

            // delta*/h ratio (DSTARH in Fortran) - used for peak Strouhal
            Real dstarh = 1.0 / hdstar;

            // Skip if bluntness is negligible
            if (hdstar < 0.01 || h <= 0.0)
            {
                result.overall_spl = -100.0;
                return true;
            }

            // High-frequency directivity (DBARH in Fortran)
            Real Dh = Directivity::high_frequency(mach, config.observer_theta, config.observer_phi);

            // Compute peak Strouhal number (STPEAK in Fortran)
            // ATERM = .212 - .0045 * PSI
            Real aterm = 0.212 - 0.0045 * psi;
            Real stpeak;
            if (hdstar >= 0.2)
            {
                stpeak = aterm / (1.0 + 0.235 * dstarh - 0.0132 * dstarh * dstarh);
            }
            else
            {
                stpeak = 0.1 * hdstar + 0.095 - 0.00243 * psi;
            }

            // Compute G4 scaled spectrum level
            Real G4 = G4_function(hdstar, psi);

            // Scaling term: 10 * log10(M^5.5 * H * DBARH * L / R^2)
            // Note: Fortran uses M**5.5, not M**5
            Real scale = 10.0 * std::log10(std::pow(mach, 5.5) * h * Dh * span /
                                           (distance * distance));

            for (size_t i = 0; i < num_freq; ++i)
            {
                Real freq = frequencies[i];

                // Strouhal number based on TE thickness (STPPP in Fortran)
                Real stppp = freq * h / velocity;

                // ETA = log10(St / St_peak)
                Real eta = std::log10(stppp / stpeak);

                // G5 at PSI=14 degrees (G514 in Fortran)
                Real hdstarl = hdstar;
                Real G514 = G5_function(hdstarl, eta);

                // G5 at PSI=0 degrees (G50 in Fortran)
                // HDSTARP = 6.724 * HDSTAR**2 - 4.019*HDSTAR + 1.107
                Real hdstarp = 6.724 * hdstar * hdstar - 4.019 * hdstar + 1.107;
                Real G50 = G5_function(hdstarp, eta);

                // Interpolate G5 based on PSI angle
                // G5 = G50 + 0.0714 * PSI * (G514 - G50)
                Real G5 = G50 + 0.0714 * psi * (G514 - G50);

                // Clamp G5 to be <= 0
                if (G5 > 0.0)
                {
                    G5 = 0.0;
                }

                // Additional check: G5 cannot exceed F4TEMP (G5 at hdstar=0.25)
                Real F4TEMP = G5_function(0.25, eta);
                if (G5 > F4TEMP)
                {
                    G5 = F4TEMP;
                }

                // Final SPL: SPLBLNT = G4 + G5 + SCALE
                result.spl[i] = G4 + G5 + scale;
            }

            result.overall_spl = MathUtils::compute_OASPL(result.spl);

            return true;
        }

    } // namespace noise
} // namespace bladenoise
