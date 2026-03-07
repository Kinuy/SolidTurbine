#pragma once

#include "bladenoise/airfoil/IBoundaryLayerCalculator.h"
#include "bladenoise/math/Spline.h"
#include <vector>

namespace bladenoise {
namespace airfoil {

class XfoilBoundaryLayerCalculator : public IBoundaryLayerCalculator {
public:
    XfoilBoundaryLayerCalculator();
    ~XfoilBoundaryLayerCalculator();

    bool calculate(
        const io::AirfoilData& airfoil,
        const ProjectConfig& config,
        BoundaryLayerState& upper_bl,
        BoundaryLayerState& lower_bl) override;

    std::string get_error() const override { return error_message_; }

    // Access to inviscid solution results
    const RealVector& get_cp() const { return cp_; }
    const RealVector& get_q_inv() const { return q_inv_; }
    Real get_cl() const { return cl_; }

private:
    // Boundary layer side data structure
    struct BLSide {
        int n_bl = 0;           // Number of BL stations
        int i_trans = -1;       // Transition station index
        Real x_trans = 1.0;     // Transition x/c location

        RealVector x;           // x/c at each station
        RealVector s;           // Arc length at each station
        RealVector ue;          // Edge velocity (normalized by Uinf)
        RealVector theta;       // Momentum thickness (normalized by chord)
        RealVector dstar;       // Displacement thickness (normalized by chord)
        RealVector cf;          // Skin friction coefficient
        RealVector h;           // Shape factor H = delta*/theta
        RealVector re_theta;    // Reynolds number based on theta
        RealVector ctau;        // Shear stress coefficient (turbulent)
        std::vector<bool> turb; // Turbulent flag per station
    };

    // Geometry setup
    bool setup_geometry(const io::AirfoilData& airfoil);

    // Inviscid solution: linear-strength vortex panel method
    bool solve_inviscid(Real alpha);

    // Boundary layer solution
    bool solve_boundary_layer(const ProjectConfig& config);
    bool march_bl_side(BLSide& side, Real x_trip, bool is_upper);

    // Viscous-inviscid coupling iteration
    bool viscous_inviscid_iteration(
        const io::AirfoilData& airfoil,
        const ProjectConfig& config,
        int max_iter = 5);

    // Closure relations
    void laminar_closure(
        Real h, Real re_theta, Real mach_e,
        Real& cf, Real& cd, Real& h_star, Real& h_star_h);

    void turbulent_closure(
        Real h, Real re_theta, Real mach_e, Real ctau,
        Real& cf, Real& cd, Real& h_star, Real& ctau_eq);

    // Transition prediction (e^N method)
    Real amplification_rate(Real h, Real re_theta) const;

    // Geometry data
    int n_points_ = 0;
    int le_index_ = 0;
    RealVector x_coords_;
    RealVector y_coords_;
    RealVector s_coords_;

    // Parametric spline for airfoil geometry
    math::ParametricSpline2D airfoil_spline_;

    // Inviscid solution data
    Real alpha_ = 0.0;      // Angle of attack (radians)
    Real cl_ = 0.0;         // Lift coefficient
    RealVector gamma_;       // Vortex strength at each node
    RealVector q_inv_;       // Inviscid velocity magnitude at each node
    RealVector cp_;          // Pressure coefficient at each node

    // Boundary layer data
    Real reynolds_ = 0.0;
    Real mach_ = 0.0;
    Real n_crit_ = 9.0;     // Critical amplification factor (e^N)

    BLSide upper_;           // Upper (suction) surface BL
    BLSide lower_;           // Lower (pressure) surface BL

    std::string error_message_;
};

}  // namespace airfoil
}  // namespace bladenoise
