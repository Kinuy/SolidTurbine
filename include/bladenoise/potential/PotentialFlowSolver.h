#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"
#include "bladenoise/io/IOTypes.h"
#include <Eigen/Dense>
#include <memory>
#include <string>
#include <vector>

namespace bladenoise {
namespace potential {

class PotentialFlowSolver {
public:
    static constexpr int NUM_GAUSS_POINTS = 4;
    static constexpr int DEFAULT_POINTS_PER_STREAMLINE = 500;

    explicit PotentialFlowSolver(int num_panels = 200);

    // Setup geometry from airfoil data and config
    bool setup_geometry(const io::AirfoilData& airfoil, const ProjectConfig& config);

    // Solve the potential flow
    bool solve();

    // Compute streamlines
    bool calculate_streamlines(int num_streamlines, Real streamline_spacing,
                              io::StreamlineData& streamlines);

    // Query results
    Real get_pressure_coefficient(Real s) const;
    Real get_lift_coefficient() const { return lift_coefficient_; }

    std::string get_error() const { return error_message_; }

private:
    // Initialization
    void initialize_constants();
    void setup_gauss_quadrature();
    void setup_derivative_coefficients();

    // Panel method
    void build_influence_matrix();
    void setup_rhs();
    bool solve_system();
    void compute_pressure_distribution();

    // Panel influence calculation
    void compute_panel_influence(Real x1, Real x2, Real s1, Real s2,
                                Real& herm1, Real& herm2,
                                Real& herm3, Real& herm4);

    // Wake contribution
    void compute_wake_contribution(Real x1, Real x2, Real y1, Real y2,
                                  Real ywinf1, Real ywinf2,
                                  Real n1, Real n2,
                                  Real& dipok, Real& v1, Real& v2,
                                  Real& dv1d1, Real& dv1d2,
                                  Real& dv2d1, Real& dv2d2);

    // Streamline computation
    void compute_streamlines(int npath, Real dpath, io::StreamlineData& streamlines);
    void integrate_streamline(Real dt, RealVector& str1, RealVector& str2,
                             RealVector& ps);

    // Spline utilities (matching original Fortran SPL_P, SPL_PP, SPL_EX, SPL_EX1)
    void spline_setup(const RealVector& x, const RealVector& y, int n, RealVector& d2y);
    void spline_setup_wrong_implemented(const RealVector& x, const RealVector& y,
                                        int n, RealVector& d2y);
    void spline_setup_natural(const RealVector& x, const RealVector& y,
                             int n, Real yp1, Real ypn, RealVector& d2y) const;
    void spline_search(const RealVector& x, int n, Real xval,
                      int& khi, int& klo) const;
    void spline_interp(const RealVector& x, const RealVector& y,
                      const RealVector& d2y, int n, Real xval,
                      Real& yval, Real& dydx, int& khi, int& klo) const;

    // Panel method dimensions
    int n_;     // Number of flow panels
    int na_;    // Number of acoustic panels
    int nstr_;  // Points per streamline
    int ng_;    // Number of Gauss quadrature points

    // Flow panel geometry
    RealVector swork_;    // Arc-length parameter
    RealVector yc1_;      // x-coordinates of panel nodes
    RealVector yc2_;      // y-coordinates of panel nodes
    RealVector nc1_, nc2_; // Normal components
    RealVector tc1_, tc2_; // Tangent components
    RealVector d2yc1_, d2yc2_; // Spline second derivatives
    RealVector ds_;       // Panel lengths

    // Acoustic panel geometry
    RealVector sworkat_;
    RealVector yc1at_, yc2at_;
    RealVector nc1at_, nc2at_;
    RealVector tc1at_, tc2at_;
    RealVector d2yc1at_, d2yc2at_;

    // Solution arrays
    RealVector pots_;     // Surface potential
    RealVector d2pots_;   // Spline of potential

    // Linear system
    Eigen::MatrixXd Kern_;
    Eigen::VectorXd rhs_;
    std::vector<int> ipiv_;
    std::unique_ptr<Eigen::PartialPivLU<Eigen::MatrixXd>> lu_solver_;

    // Gaussian quadrature
    std::vector<RealVector> td_;  // Quadrature points
    std::vector<RealVector> Ad_;  // Quadrature weights

    // Finite difference coefficients
    std::vector<RealVector> dst_;

    // Wake geometry
    Real ywinf1_ = 0.0, ywinf2_ = 0.0;  // Wake point at infinity
    Real ywn1_ = 0.0, ywn2_ = 0.0;      // Wake normal

    // Results
    Real lift_coefficient_ = 0.0;
    Real angle_of_attack_ = 0.0;

    // State
    bool geometry_initialized_ = false;
    bool solution_computed_ = false;

    std::string error_message_;
};

}  // namespace potential
}  // namespace bladenoise
