#pragma once

#include "bladenoise/airfoil/IBoundaryLayerCalculator.h"

namespace bladenoise {
namespace airfoil {

class BPMBoundaryLayerCalculator : public IBoundaryLayerCalculator {
public:
    bool calculate(
        const io::AirfoilData& airfoil,
        const ProjectConfig& config,
        BoundaryLayerState& upper_bl,
        BoundaryLayerState& lower_bl) override;

    std::string get_error() const override { return error_message_; }

    // Static method for use by other calculators
    // Computes BL thickness (delta), displacement thickness (delta*_p)
    // for pressure side from BPM correlations (NASA RP-1218)
    static void calculate_thicknesses(
        Real chord, Real velocity, Real alpha, TripConfig trip,
        Real& delta_p, Real& delta_s, Real& delta_p_star,
        Real c0, Real visc);

    // Compute suction-side displacement thickness from separate BPM correlations
    static void calculate_displacement_thickness_suction(
        Real chord, Real velocity, Real alpha, TripConfig trip,
        Real c0, Real visc, Real& delta_s_star);

private:
    std::string error_message_;
};

}  // namespace airfoil
}  // namespace bladenoise
