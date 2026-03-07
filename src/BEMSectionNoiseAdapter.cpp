/**
 * @file BEMSectionNoiseAdapter.cpp
 * @brief Maps BEMPostprocessResult + TurbineGeometry → SectionNoiseInput.
 */
#define _USE_MATH_DEFINES
#include "BEMSectionNoiseAdapter.h"
#include "AirfoilDataAdapter.h"
#include "TurbineGeometry.h"
#include <cmath>
#include <numbers>

BEMSectionNoiseAdapter::BEMSectionNoiseAdapter(double observer_distance,
                                               double observer_theta,
                                               double observer_phi,
                                               double turbulence_intensity,
                                               double turbulence_length_scale)
    : observer_distance_(observer_distance)
    , observer_theta_(observer_theta)
    , observer_phi_(observer_phi)
    , turbulence_intensity_(turbulence_intensity)
    , turbulence_length_scale_(turbulence_length_scale)
{}

std::vector<SectionNoiseInput> BEMSectionNoiseAdapter::Build(
    BEMPostprocessResult const &pp,
    TurbineGeometry const      *turbine,
    ISimulationConfig const    &sim_config,
    double                      /*vinf*/) const
{
    if (!turbine) return {};

    const std::size_t n_sec = turbine->num_sections();
    if (pp.alpha_eff.size() < n_sec) return {};

    std::vector<SectionNoiseInput> inputs;
    inputs.reserve(n_sec);

    for (std::size_t i = 0; i < n_sec; ++i)
    {
        SectionNoiseInput inp;
        inp.section_index = i;

        // ── Atmospheric constants (from ISimulationConfig) ────────────────────
        inp.speed_of_sound       = sim_config.speed_of_sound();
        inp.kinematic_viscosity  = sim_config.kinematic_viscosity();
        inp.air_density          = sim_config.air_density();

        // ── Geometry ──────────────────────────────────────────────────────────
        inp.chord = turbine->chord(i);

        // Element span: midpoint-based dr
        if (n_sec > 1)
        {
            double r_low = (i == 0)
                ? turbine->radius(0)
                : 0.5 * (turbine->radius(i - 1) + turbine->radius(i));
            double r_high = (i == n_sec - 1)
                ? turbine->radius(n_sec - 1)
                : 0.5 * (turbine->radius(i) + turbine->radius(i + 1));
            inp.span = r_high - r_low;
        }
        else
        {
            inp.span = turbine->radius(0);
        }

        // ── Flow state ────────────────────────────────────────────────────────
        // Local velocity from element_thrust / element_torque back-calculation
        // is not readily available here; use the BEM-derived per-section velocity
        // magnitude reconstructed from Mach number (if section vectors are present).
        // Fallback: use the first section value as a representative velocity.
        // A richer solution would require passing the NingSolver reference.
        // We use: V_rel ~ sqrt((U_ax*(1-a))^2 + (Omega*r*(1+a'))^2)
        // which equals: V_rel = c0 * Mach(i)  — but Mach is on NingSolver, not pp.
        // Instead we derive from local thrust / chord / span / density approximation.
        // Best available: use element_thrust and element_torque with BEM geometry.
        // For now use a conservative approach: store alpha and let the caller
        // override velocity if they have NingSolver access.
        inp.velocity  = 0.0;   // caller must fill from NingSolver::LocalFlowVel(i)
        inp.alpha_deg = pp.alpha_eff[i] * (180.0 / std::numbers::pi);
        inp.mach      = 0.0;   // caller fills from NingSolver::LocalMachNumber(i)
        inp.reynolds  = 0.0;   // caller fills from NingSolver::LocalReynoldsNumber(i)

        // ── Trailing-edge geometry ────────────────────────────────────────────
        // thickness() returns absolute thickness [m]; TE thickness ≈ 1% of that
        inp.te_thickness = turbine->thickness(i) * 0.01; // heuristic default
        inp.te_angle_deg = 14.0;                          // BPM default

        // ── Thickness fractions for TI noise ─────────────────────────────────
        // TurbineGeometry provides relative thickness (t/c * 100) via relative_thickness()
        // We approximate t/c at 1% and 10% from the overall t/c ratio.
        const double tc = turbine->thickness(i) / inp.chord;  // t/c [-]
        inp.thickness_1_percent  = tc * 0.6;  // heuristic: ~60% of max at 1% chord
        inp.thickness_10_percent = tc * 0.9;  // heuristic: ~90% of max at 10% chord

        // ── Turbulence ────────────────────────────────────────────────────────
        inp.turbulence_intensity    = turbulence_intensity_;
        inp.turbulence_length_scale = turbulence_length_scale_;

        // ── Observer ─────────────────────────────────────────────────────────
        inp.observer_distance = observer_distance_;
        inp.observer_theta    = observer_theta_;
        inp.observer_phi      = observer_phi_;

        // ── Xfoil BL: attach normalised airfoil coordinates ──────────────────
        // TurbineGeometry::airfoilGeometry(i) returns the normalised (0..1)
        // AirfoilGeometryData for section i via BladeGeometrySection.
        // getCoordinates() is safe because applyScalingWithChordAndMaxThickness
        // writes into scaledCoordinates and leaves coordinates untouched.
        {
        const AirfoilGeometryData *ag = turbine->airfoilGeometry(i);
            if (ag && AirfoilDataAdapter::IsUsableForXfoil(*ag))
            {
                auto adata = std::make_shared<bladenoise::io::AirfoilData>();
                AirfoilDataAdapter::Convert(*ag, *adata);
                inp.airfoil_data = std::move(adata);
                //TODO: Check number of airfoil points!
            }
        }

        inputs.push_back(inp);       
    }

    return inputs;
}
