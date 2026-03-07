#include "bladenoise/airfoil/XfoilBoundaryLayerCalculator.h"
#include "bladenoise/core/Constants.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <Eigen/Dense>

namespace bladenoise {
namespace airfoil {

using namespace constants;

XfoilBoundaryLayerCalculator::XfoilBoundaryLayerCalculator() = default;
XfoilBoundaryLayerCalculator::~XfoilBoundaryLayerCalculator() = default;

// ============================================================================
// Geometry Setup
// ============================================================================

bool XfoilBoundaryLayerCalculator::setup_geometry(const io::AirfoilData& airfoil) {
    n_points_ = static_cast<int>(airfoil.num_points);
    if (n_points_ < 10) {
        error_message_ = "Too few airfoil points";
        return false;
    }

    x_coords_ = airfoil.x;
    y_coords_ = airfoil.y;

    // Arc-length parametrization
    s_coords_.resize(n_points_);
    s_coords_[0] = 0.0;
    for (int i = 1; i < n_points_; ++i) {
        Real dx = x_coords_[i] - x_coords_[i-1];
        Real dy = y_coords_[i] - y_coords_[i-1];
        s_coords_[i] = s_coords_[i-1] + std::sqrt(dx*dx + dy*dy);
    }

    // Leading edge = minimum x
    le_index_ = 0;
    Real x_min = x_coords_[0];
    for (int i = 1; i < n_points_; ++i) {
        if (x_coords_[i] < x_min) {
            x_min = x_coords_[i];
            le_index_ = i;
        }
    }

    return true;
}

// ============================================================================
// Linear-Strength Vortex Panel Method
//
// Each panel j (node j → node j+1) carries a vortex sheet whose strength
// varies linearly:
//       γ(t) = γ_j·(1−t) + γ_{j+1}·t      t ∈ [0, 1]
//
// There are N+1 unknowns (N = n_points_−1 panels, N+1 nodes).
// Equations:
//   rows 0..N−1 : V_total · n̂_i = 0  at panel-i midpoint   (no-penetration)
//   row  N      : γ[0] + γ[N] = 0                           (Kutta condition)
//
// The influence coefficients are evaluated with 8-point Gauss-Legendre
// quadrature, which is exact for smooth kernels and avoids all the
// analytical-formula sign-convention pitfalls.
// ============================================================================

// 8-point Gauss-Legendre on [−1, 1]
static const int NGP = 8;
static const double GP_T[NGP] = {
    -0.96028985649753623, -0.79666647741362674,
    -0.52553240991632899, -0.18343464249564980,
     0.18343464249564980,  0.52553240991632899,
     0.79666647741362674,  0.96028985649753623
};
static const double GP_W[NGP] = {
    0.10122853629037626, 0.22238103445337447,
    0.31370664587788729, 0.36268378337836198,
    0.36268378337836198, 0.31370664587788729,
    0.22238103445337447, 0.10122853629037626
};

bool XfoilBoundaryLayerCalculator::solve_inviscid(Real alpha) {
    alpha_ = alpha * DEG_TO_RAD;

    const int n  = n_points_ - 1;   // panels
    const int N  = n_points_;        // nodes = unknowns

    // ------------------------------------------------------------------
    // Panel geometry
    // ------------------------------------------------------------------
    std::vector<Real> xm(n), ym(n);
    std::vector<Real> dxp(n), dyp(n), plen(n);
    std::vector<Real> tx(n), ty(n), nx(n), ny(n);

    for (int i = 0; i < n; ++i) {
        dxp[i] = x_coords_[i+1] - x_coords_[i];
        dyp[i] = y_coords_[i+1] - y_coords_[i];
        plen[i] = std::sqrt(dxp[i]*dxp[i] + dyp[i]*dyp[i]);
        if (plen[i] < 1e-14) plen[i] = 1e-14;

        tx[i] = dxp[i] / plen[i];
        ty[i] = dyp[i] / plen[i];
        nx[i] =  ty[i];             // rotate tangent +90°
        ny[i] = -tx[i];

        xm[i] = 0.5 * (x_coords_[i] + x_coords_[i+1]);
        ym[i] = 0.5 * (y_coords_[i] + y_coords_[i+1]);
    }

    // Fix normal orientation so normals point OUTWARD (away from interior).
    // Strategy: compute the airfoil centroid, then check whether normals
    // point away from or toward it.  If the majority point inward → flip.
    {
        Real cx = 0.0, cy = 0.0;
        for (int i = 0; i < n_points_; ++i) { cx += x_coords_[i]; cy += y_coords_[i]; }
        cx /= n_points_;  cy /= n_points_;

        int outward_count = 0;
        for (int i = 0; i < n; ++i) {
            // Vector from panel midpoint toward centroid
            Real dx_c = cx - xm[i];
            Real dy_c = cy - ym[i];
            // If normal points away from centroid, dot product < 0
            if (nx[i]*dx_c + ny[i]*dy_c < 0.0)
                ++outward_count;
        }
        // If fewer than half are outward, flip all normals
        if (outward_count < n / 2) {
            for (int i = 0; i < n; ++i) { nx[i] = -nx[i]; ny[i] = -ny[i]; }
        }
    }

    Real cos_a = std::cos(alpha_);
    Real sin_a = std::sin(alpha_);

    // ------------------------------------------------------------------
    // Influence matrix  A · γ = b          (size N × N)
    //
    //  For i = 0 … N−2  (no-penetration at panel-i midpoint):
    //    Σ_k  A(i,k)·γ_k  =  −V∞ · n̂_i
    //
    //  Panel j contributes to columns j and j+1 because
    //    γ(t) = γ_j·(1−t) + γ_{j+1}·t
    //
    //  For each Gauss point on panel j we compute the Biot-Savart
    //  kernel for a point vortex at that location, multiply by the
    //  linear basis function, and accumulate the weighted result into
    //  A(i, j) and A(i, j+1).
    // ------------------------------------------------------------------

    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(N, N);
    Eigen::VectorXd b = Eigen::VectorXd::Zero(N);

    for (int i = 0; i < n; ++i) {                       // control points
        b(i) = -(cos_a * nx[i] + sin_a * ny[i]);

        for (int j = 0; j < n; ++j) {                   // source panels
            Real x1j = x_coords_[j], y1j = y_coords_[j];

            Real cn_j0 = 0.0, cn_j1 = 0.0;

            for (int g = 0; g < NGP; ++g) {
                Real t  = 0.5 * (1.0 + GP_T[g]);        // [0,1]
                Real wt = 0.5 * GP_W[g] * plen[j];      // weight × Jacobian

                Real xs = x1j + t * dxp[j];
                Real ys = y1j + t * dyp[j];

                Real rx = xm[i] - xs;
                Real ry = ym[i] - ys;
                Real r2 = rx*rx + ry*ry;
                if (r2 < 1e-20) continue;

                // Point-vortex Biot-Savart:  v = γ/(2π r²) (−Δy, Δx)
                Real fac = wt / (TWO_PI * r2);
                Real vn  = (-ry * nx[i] + rx * ny[i]) * fac;

                cn_j0 += vn * (1.0 - t);
                cn_j1 += vn * t;
            }

            A(i, j)   += cn_j0;
            A(i, j+1) += cn_j1;
        }
    }

    // Kutta condition  γ[0] + γ[N−1] = 0
    A.row(N-1).setZero();
    A(N-1, 0)   = 1.0;
    A(N-1, N-1) = 1.0;
    b(N-1) = 0.0;

    // Solve
    Eigen::VectorXd gv = A.colPivHouseholderQr().solve(b);

    gamma_.resize(N);
    for (int i = 0; i < N; ++i) gamma_[i] = gv(i);

    // ------------------------------------------------------------------
    // Tangential velocity at each panel midpoint
    //   q_i = V∞·t̂_i  +  Σ_j ∫₀¹ γ_j(t)·K_t(panel_j → cp_i) dt
    // ------------------------------------------------------------------
    std::vector<Real> q_panel(n);

    for (int i = 0; i < n; ++i) {
        Real qt = cos_a * tx[i] + sin_a * ty[i];   // freestream

        for (int j = 0; j < n; ++j) {
            Real x1j = x_coords_[j], y1j = y_coords_[j];

            for (int g = 0; g < NGP; ++g) {
                Real t  = 0.5 * (1.0 + GP_T[g]);
                Real wt = 0.5 * GP_W[g] * plen[j];

                Real xs = x1j + t * dxp[j];
                Real ys = y1j + t * dyp[j];

                Real rx = xm[i] - xs;
                Real ry = ym[i] - ys;
                Real r2 = rx*rx + ry*ry;
                if (r2 < 1e-20) continue;

                Real gam_local = gamma_[j] * (1.0 - t) + gamma_[j+1] * t;
                Real fac = wt / (TWO_PI * r2);
                // tangential component at panel i
                qt += gam_local * (-ry * tx[i] + rx * ty[i]) * fac;
            }
        }
        q_panel[i] = qt;
    }

    // Map to nodes (average of adjacent panels)
    q_inv_.resize(N);
    cp_.resize(N);
    for (int i = 0; i < N; ++i) {
        if (i == 0)
            q_inv_[i] = std::abs(q_panel[0]);
        else if (i == N - 1)
            q_inv_[i] = std::abs(q_panel[n-1]);
        else
            q_inv_[i] = std::abs(0.5 * (q_panel[i-1] + q_panel[i]));

        q_inv_[i] = std::max(q_inv_[i], 1e-6);
        cp_[i] = 1.0 - q_inv_[i] * q_inv_[i];
    }

    // ------------------------------------------------------------------
    // Lift coefficient
    // ------------------------------------------------------------------
    Real xmin = *std::min_element(x_coords_.begin(), x_coords_.end());
    Real xmax = *std::max_element(x_coords_.begin(), x_coords_.end());
    Real chord = std::max(xmax - xmin, 1e-10);

    // Kutta-Joukowski:  Γ = ∫ γ ds  ⇒  CL = 2Γ/(V∞·c)
    Real circ = 0.0;
    for (int j = 0; j < n; ++j)
        circ += 0.5 * (gamma_[j] + gamma_[j+1]) * plen[j];
    Real cl_kj = 2.0 * circ / chord;

    // Cp integration: CL = -(1/c) ∮ Cp (n̂ · ê_L) ds
    // where ê_L = (-sin α, cos α) is the lift direction
    Real cl_cp = 0.0;
    for (int i = 0; i < n; ++i) {
        Real cp_mid = 0.5 * (cp_[i] + cp_[i+1]);
        Real lift_proj = nx[i] * (-sin_a) + ny[i] * cos_a;
        cl_cp -= cp_mid * lift_proj * plen[i] / chord;
    }

    // The Cp integration sign is always correct because normals were
    // verified outward via centroid check.  K-J magnitude is more
    // accurate (Cp loses sign through |q|), so take magnitude from K-J
    // and sign from Cp integration.
    cl_ = std::copysign(std::abs(cl_kj), cl_cp);

    // If Cp integration is near zero (symmetric, alpha≈0), trust K-J as-is
    if (std::abs(cl_cp) < 0.01)
        cl_ = cl_kj;

    //TODO: debug output: std::cout << "  Panel method CL (Kutta-Joukowski): " << cl_kj << "\n";
    //TODO: debug output: std::cout << "  Panel method CL (Cp integration):  " << cl_cp << "\n";
    //TODO: debug output: std::cout << "  Panel method CL (combined):        " << cl_ << "\n";

    return true;
}

// ============================================================================
// Boundary Layer Closure Relations
// ============================================================================

void XfoilBoundaryLayerCalculator::laminar_closure(
    Real h, Real re_theta, Real mach_e,
    Real& cf, Real& cd, Real& h_star, Real& h_star_h)
{
    Real hk = std::max(1.05, std::min(h, 10.0));

    if (hk < 5.5)
        cf = (0.0727 * std::pow(5.5 - hk, 3.0) / (hk + 1.0) - 0.07) / re_theta;
    else
        cf = (0.015 * (1.0 - 1.0 / (hk - 4.5)) - 0.07) / re_theta;
    cf = std::max(cf, -0.1 / re_theta);

    if (hk < 4.0)
        cd = (0.207 + 0.00205 * std::pow(4.0 - hk, 5.5)) / re_theta;
    else {
        cd = (0.207 - 0.003 * (hk - 4.0) * (hk - 4.0)) / re_theta;
        cd = std::max(cd, 0.10 / re_theta);
    }

    if (hk < 4.0)
        h_star = 1.515 + 0.076 * (4.0 - hk) * (4.0 - hk) / hk;
    else
        h_star = 1.515 + 0.040 * (hk - 4.0) * (hk - 4.0) / hk;

    if (hk < 4.0)
        h_star_h = -0.076 * (2.0 * (4.0 - hk) * hk + (4.0 - hk) * (4.0 - hk)) / (hk * hk);
    else
        h_star_h = 0.040 * (2.0 * (hk - 4.0) * hk - (hk - 4.0) * (hk - 4.0)) / (hk * hk);

    Real fc = 1.0 + 0.2 * mach_e * mach_e;
    cf /= fc;
    cd /= fc;
}

void XfoilBoundaryLayerCalculator::turbulent_closure(
    Real h, Real re_theta, Real mach_e, Real /*ctau*/,
    Real& cf, Real& cd, Real& h_star, Real& ctau_eq)
{
    Real hk = std::max(1.00005, std::min(h, 6.0));
    Real fc = std::sqrt(1.0 + 0.2 * mach_e * mach_e);

    Real grt = std::max(std::log(re_theta / fc), 3.0);
    Real gex = -1.74 - 0.31 * hk;
    Real arg = std::max(-1.33 * hk, -20.0);
    Real thk = std::tanh(4.0 - hk / 0.875);

    Real cfo = 0.3 * std::exp(arg) * std::pow(grt / 2.3026, gex);
    cf = (cfo + 1.1e-4 * (thk - 1.0)) / fc;
    cf = std::max(cf, 1e-6);

    Real hk_1 = hk - 1.0;
    cd = cf / 2.0 * (1.0 + 0.995 * hk_1 * hk_1 / hk) +
         0.01 * std::pow(std::max(hk_1, 0.0), 2.0) / re_theta;

    h_star = 1.505 + 4.0 / (re_theta + 200.0) + 0.165 - 1.6 / (hk + 0.5);
    h_star = std::max(h_star, 1.01);

    Real hk_eq = (re_theta > 200.0)
        ? 1.0 + 6.0 / (re_theta - 100.0 + std::abs(re_theta - 100.0) + 20.0)
        : 1.3;

    if (hk > hk_eq) {
        ctau_eq = cf / 2.0 * (hk - hk_eq) * 0.015 / (cf / 2.0 + 0.001);
        ctau_eq = std::max(ctau_eq, 0.0002);
    } else {
        ctau_eq = 0.0002;
    }
}

Real XfoilBoundaryLayerCalculator::amplification_rate(Real h, Real re_theta) const {
    Real hk = std::max(h, 1.05);

    Real log_re_crit;
    if (hk < 1.1)
        log_re_crit = 12.0;
    else {
        Real ha = std::min(hk, 4.0);
        log_re_crit = (1.415 / (ha - 1.0) - 0.489) *
                      std::tanh(20.0 / (ha - 1.0) - 12.9) +
                      3.295 / (ha - 1.0) + 0.44;
    }

    if (re_theta < std::exp(log_re_crit))
        return 0.0;

    Real dn_dre;
    if (hk <= 3.5)
        dn_dre = 0.028 * (hk - 1.0) - 0.0345 * std::exp(-3.87 / (hk - 1.0));
    else
        dn_dre = 0.086 * (hk - 1.0) - 0.25;

    return std::max(dn_dre, 0.0);
}

// ============================================================================
// Boundary Layer Marching
// ============================================================================

bool XfoilBoundaryLayerCalculator::march_bl_side(
    BLSide& side, Real x_trip, bool is_upper)
{
    const int n_max = 300;
    side.x.resize(n_max);      side.s.resize(n_max);
    side.ue.resize(n_max);     side.theta.resize(n_max);
    side.dstar.resize(n_max);  side.cf.resize(n_max);
    side.h.resize(n_max);      side.re_theta.resize(n_max);
    side.ctau.resize(n_max);   side.turb.resize(n_max, false);
    side.x_trans = 1.0;        side.i_trans = -1;

    int i_start, i_end, i_step;
    if (is_upper) { i_start = le_index_; i_end = 0;             i_step = -1; }
    else          { i_start = le_index_; i_end = n_points_ - 1; i_step =  1; }

    Real theta = 0.29004 / std::sqrt(reynolds_);
    Real h     = 2.591;
    Real ctau  = 0.0;
    Real n_amp = 0.0;
    bool turbulent = false;

    int  n_bl   = 0;
    Real s_prev = s_coords_[i_start];

    for (int idx = i_start; idx != i_end + i_step; idx += i_step) {
        if (n_bl >= n_max - 1 || idx < 0 || idx >= n_points_) break;
        if (idx >= static_cast<int>(q_inv_.size())) continue;

        Real x  = x_coords_[idx];
        Real s  = s_coords_[idx];
        Real ue = std::max(q_inv_[idx], 0.001);

        Real ds = std::abs(s - s_prev);
        if (ds < 1e-10 && n_bl > 0) { s_prev = s; continue; }

        side.x[n_bl]  = x;
        side.s[n_bl]  = s;
        side.ue[n_bl] = ue;

        Real re_th = std::max(reynolds_ * ue * theta, 1.0);
        Real me    = mach_ * ue;

        Real cf_loc, cd_loc, h_star, h_star_h;
        Real ctau_eq = 0.0;

        // ---------- laminar region ----------
        if (!turbulent) {
            laminar_closure(h, re_th, me, cf_loc, cd_loc, h_star, h_star_h);

            Real dre_ds = 0.0;
            if (n_bl > 0 && ds > 1e-10)
                dre_ds = (re_th - side.re_theta[n_bl-1]) / ds;

            Real dn_dre = amplification_rate(h, re_th);
            Real dn_ds  = (dre_ds > 0)
                ? dn_dre * dre_ds
                : dn_dre * reynolds_ * ue * std::max(cf_loc / 2.0, 1e-6);
            n_amp += dn_ds * ds;
            n_amp  = std::max(n_amp, 0.0);

            if (n_amp >= n_crit_ || x >= x_trip) {
                turbulent    = true;
                side.i_trans = n_bl;
                side.x_trans = x;
                ctau = std::max(cf_loc, 0.001) * 0.5;
                h    = std::min(h, 2.4);
            }
        }

        // ---------- turbulent region ----------
        if (turbulent) {
            turbulent_closure(h, re_th, me, ctau, cf_loc, cd_loc, h_star, ctau_eq);

            if (ds > 1e-10) {
                Real rate = 5.0 * std::sqrt(std::max(ctau, 0.0)) *
                            (ctau_eq - ctau) / theta;
                ctau += rate * ds;
                ctau  = std::clamp(ctau, 0.00005, 0.3);
            }
        }

        side.theta[n_bl]    = theta;
        side.dstar[n_bl]    = theta * h;
        side.cf[n_bl]       = cf_loc;
        side.h[n_bl]        = h;
        side.re_theta[n_bl] = re_th;
        side.ctau[n_bl]     = ctau;
        side.turb[n_bl]     = turbulent;

        // ---------- march ----------
        if (n_bl > 0 && ds > 1e-10) {
            Real ue_prev = std::max(side.ue[n_bl-1], 0.001);
            Real due_ds  = (ue - ue_prev) / ds;
            Real me2     = me * me;

            Real dtheta_ds = cf_loc / 2.0 -
                             theta / ue * (h + 2.0 - me2) * due_ds;

            if (turbulent) {
                Real beta = -theta * theta / (cf_loc / 2.0 + 1e-8) * due_ds / ue;
                Real h_eq = std::clamp(1.4 + 0.8 * std::max(beta, 0.0), 1.2, 3.5);

                Real dH_ds = (2.0*cd_loc/theta - h_star*cf_loc/(2.0*theta)
                             - (h_star-1.0)/theta * dtheta_ds) / (h_star_h + 1e-6);
                Real dH_max = 5.0 * std::abs(h - h_eq);
                dH_ds = std::clamp(dH_ds, -dH_max, dH_max);
                h += dH_ds * ds;
            } else {
                Real lam = theta * theta * due_ds / (ue / reynolds_);
                Real h_eq;
                if (lam < 0.0)
                    h_eq = 2.088 + 0.0731 / (lam + 0.14);
                else
                    h_eq = 2.61 - 3.75*lam + 5.24*lam*lam;
                h_eq = std::clamp(h_eq, 2.0, 4.5);

                Real relax = std::min(ds / (10.0*theta + 1e-10), 0.5);
                h += relax * (h_eq - h);
            }

            theta += dtheta_ds * ds;
            theta  = std::max(theta, 1e-10);
            h = turbulent ? std::clamp(h, 1.05, 4.0)
                          : std::clamp(h, 1.5,  5.0);
            if (turbulent && cf_loc < 0.0) h = std::max(h, 2.5);
        }

        s_prev = s;
        ++n_bl;
    }

    side.n_bl = n_bl;
    side.x.resize(n_bl);        side.s.resize(n_bl);
    side.ue.resize(n_bl);       side.theta.resize(n_bl);
    side.dstar.resize(n_bl);    side.cf.resize(n_bl);
    side.h.resize(n_bl);        side.re_theta.resize(n_bl);
    side.ctau.resize(n_bl);     side.turb.resize(n_bl);

    return n_bl > 5;
}

bool XfoilBoundaryLayerCalculator::solve_boundary_layer(const ProjectConfig& config) {
    reynolds_ = config.reynolds_number();
    mach_     = config.mach_number();
    n_crit_   = 9.0;

    if (!march_bl_side(upper_, config.xtr_upper, true)) {
        error_message_ = "Upper surface BL march failed";
        return false;
    }
    if (!march_bl_side(lower_, config.xtr_lower, false)) {
        error_message_ = "Lower surface BL march failed";
        return false;
    }
    return true;
}

// ============================================================================
// Viscous-Inviscid Coupling
// ============================================================================

bool XfoilBoundaryLayerCalculator::viscous_inviscid_iteration(
    const io::AirfoilData& /*airfoil*/,
    const ProjectConfig& config,
    int max_iter)
{
    for (int iter = 0; iter < max_iter; ++iter) {
        std::vector<Real> ue_u(upper_.n_bl), ue_l(lower_.n_bl);
        for (int i = 0; i < upper_.n_bl; ++i) ue_u[i] = upper_.ue[i];
        for (int i = 0; i < lower_.n_bl; ++i) ue_l[i] = lower_.ue[i];

        if (iter > 0) {
            Real ds_u = upper_.dstar[upper_.n_bl - 1];
            Real ds_l = lower_.dstar[lower_.n_bl - 1];
            Real da   = -(ds_u - ds_l) * 0.5 * RAD_TO_DEG * 0.5;
            if (!solve_inviscid(config.angle_of_attack + da)) break;
        }

        if (!solve_boundary_layer(config)) break;

        Real mc = 0.0;
        for (int i = 0; i < std::min(upper_.n_bl, (int)ue_u.size()); ++i)
            mc = std::max(mc, std::abs(upper_.ue[i] - ue_u[i]) /
                              (std::abs(ue_u[i]) + 1e-10));
        for (int i = 0; i < std::min(lower_.n_bl, (int)ue_l.size()); ++i)
            mc = std::max(mc, std::abs(lower_.ue[i] - ue_l[i]) /
                              (std::abs(ue_l[i]) + 1e-10));

        if (mc < 0.001 && iter > 0) {
            std::cout << "  V-I coupling converged in " << iter+1 << " iterations\n";
            break;
        }
    }
    return true;
}

// ============================================================================
// Main Entry Point
// ============================================================================

bool XfoilBoundaryLayerCalculator::calculate(
    const io::AirfoilData& airfoil,
    const ProjectConfig& config,
    BoundaryLayerState& upper_bl,
    BoundaryLayerState& lower_bl)
{
    //TODO: debug output: std::cout << "  XFOIL BL Calculator: Setting up geometry...\n";
    if (!setup_geometry(airfoil)) return false;

    //TODO: debug output: std::cout << "  XFOIL BL Calculator: Solving inviscid flow (linear-vortex)...\n";
    if (!solve_inviscid(config.angle_of_attack)) {
        error_message_ = "Inviscid solution failed";
        return false;
    }

    //TODO: debug output: std::cout << "  XFOIL BL Calculator: Solving boundary layer...\n";
    if (!solve_boundary_layer(config)) return false;

    //TODO: debug output: std::cout << "  XFOIL BL Calculator: Viscous-inviscid coupling...\n";
    viscous_inviscid_iteration(airfoil, config, 5);

    // Extract trailing-edge state
    auto extract_te = [&config](const BLSide& side, BoundaryLayerState& bl) {
        if (side.n_bl < 2) return;

        int ite = side.n_bl - 1;
        for (int i = side.n_bl - 1; i >= 0; --i)
            if (side.x[i] < 0.99) { ite = std::min(i+1, side.n_bl-1); break; }

        bl.displacement_thickness = side.dstar[ite] * config.chord;
        bl.momentum_thickness     = side.theta[ite] * config.chord;
        bl.shape_factor           = side.h[ite];
        bl.skin_friction          = side.cf[ite];
        bl.edge_velocity          = side.ue[ite] * config.freestream_velocity;
        bl.reynolds_theta         = side.re_theta[ite];
        bl.is_turbulent           = side.turb[ite];

        bl.boundary_layer_thickness = bl.is_turbulent
            ? bl.momentum_thickness * (3.15 + 1.72 / (std::max(bl.shape_factor, 1.1) - 1.0))
            : bl.displacement_thickness * 3.0;
    };

    extract_te(upper_, upper_bl);
    extract_te(lower_, lower_bl);

    //TODO: debug output: std::cout << "  XFOIL BL Calculator: Complete\n";
    //TODO: debug output: std::cout << "    Upper transition at x/c = " << upper_.x_trans << "\n";
    //TODO: debug output: std::cout << "    Lower transition at x/c = " << lower_.x_trans << "\n";
    //TODO: debug output: std::cout << "    CL = " << cl_ << "\n";

    return true;
}

}  // namespace airfoil
}  // namespace bladenoise
