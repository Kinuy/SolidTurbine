#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"

namespace bladenoise {
namespace noise {

class TBLTENoiseSource {
public:
    bool calculate(
        const ProjectConfig& config,
        const BoundaryLayerState& upper_bl,
        const BoundaryLayerState& lower_bl,
        const RealVector& frequencies,
        NoiseResult& result);

    // Individual noise component results (accessible after calculate())
    NoiseResult pressure_side_result;
    NoiseResult suction_side_result;
    NoiseResult separation_result;
    NoiseResult laminar_result;       // LBL-VS noise result (NEW)

private:
    // A-curve spectral shape (min/max bounds)
    Real A_min(Real a) const;
    Real A_max(Real a) const;

    // B-curve spectral shape (min/max bounds)
    Real B_min(Real b) const;
    Real B_max(Real b) const;

    // Amplitude functions
    Real compute_A0(Real rc) const;
    Real compute_B0(Real rc) const;
    Real compute_K1(Real rc) const;
    Real compute_K2(Real K1, Real rc, Real mach, Real alpha) const;
    Real compute_delta_K1(Real Re_dstar_p, Real alpha) const;

    // =========================================================
    // Laminar Boundary Layer Vortex Shedding (LBL-VS) functions
    // Corresponds to Fortran LBLVS / G1COMP subroutines
    // =========================================================

    // G1: Spectral shape function for LBL-VS (5-piece fit)
    Real G1_function(Real e) const;

    // G2: Reynolds-number-dependent amplitude for LBL-VS
    Real G2_function(Real Re_dstar) const;

    // G3: Angle of attack correction for LBL-VS
    Real G3_function(Real alpha) const;

    // Full LBL-VS noise calculation (Fortran LBLVS)
    void compute_laminar_vortex_shedding(
        const ProjectConfig& config,
        const BoundaryLayerState& upper_bl,
        const BoundaryLayerState& lower_bl,
        const RealVector& frequencies,
        NoiseResult& lam_result);
};

}  // namespace noise
}  // namespace bladenoise
