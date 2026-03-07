#include "bladenoise/potential/PotentialFlowSolver.h"
#include "bladenoise/core/Constants.h"
#include "bladenoise/math/ODEIntegrator.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace bladenoise
{
    namespace potential
    {

        using namespace constants;

        PotentialFlowSolver::PotentialFlowSolver(int num_panels)
            : n_(num_panels), na_(num_panels), nstr_(DEFAULT_POINTS_PER_STREAMLINE), ng_(NUM_GAUSS_POINTS), lift_coefficient_(0.0), angle_of_attack_(0.0), geometry_initialized_(false), solution_computed_(false)
        {
            // Allocate arrays
            swork_.resize(n_ + 1, 0.0);
            yc1_.resize(n_ + 1, 0.0);
            yc2_.resize(n_ + 1, 0.0);
            nc1_.resize(n_ + 1, 0.0);
            nc2_.resize(n_ + 1, 0.0);
            tc1_.resize(n_ + 1, 0.0);
            tc2_.resize(n_ + 1, 0.0);
            d2yc1_.resize(n_ + 1, 0.0);
            d2yc2_.resize(n_ + 1, 0.0);
            ds_.resize(n_, 0.0);
            pots_.resize(n_ + 1, 0.0);
            d2pots_.resize(n_ + 1, 0.0);

            Kern_.resize(n_ + 2, n_ + 2);
            rhs_.resize(n_ + 2);
            ipiv_.resize(n_ + 2);

            // Initialize Gaussian quadrature and derivative coefficients
            td_.resize(ng_, RealVector(ng_, 0.0));
            Ad_.resize(ng_, RealVector(ng_, 0.0));
            dst_.resize(5, RealVector(5, 0.0));

            initialize_constants();
        }

        void PotentialFlowSolver::initialize_constants()
        {
            setup_gauss_quadrature();
            setup_derivative_coefficients();
        }

        void PotentialFlowSolver::setup_gauss_quadrature()
        {
            // Gaussian quadrature points and weights for 1-4 point rules
            // td[k][ng-1] = Gauss point k for ng-point rule
            // Ad[k][ng-1] = Weight for point k

            // 1-point rule
            td_[0][0] = 0.0;
            Ad_[0][0] = 2.0;

            // 2-point rule
            td_[0][1] = -std::sqrt(1.0 / 3.0);
            td_[1][1] = std::sqrt(1.0 / 3.0);
            Ad_[0][1] = 1.0;
            Ad_[1][1] = 1.0;

            // 3-point rule
            td_[0][2] = -std::sqrt(3.0 / 5.0);
            td_[1][2] = 0.0;
            td_[2][2] = std::sqrt(3.0 / 5.0);
            Ad_[0][2] = 5.0 / 9.0;
            Ad_[1][2] = 8.0 / 9.0;
            Ad_[2][2] = 5.0 / 9.0;

            // 4-point rule
            Real sqrt30 = std::sqrt(30.0);
            td_[0][3] = -std::sqrt((15.0 + 2.0 * sqrt30) / 35.0);
            td_[1][3] = -std::sqrt((15.0 - 2.0 * sqrt30) / 35.0);
            td_[2][3] = std::sqrt((15.0 - 2.0 * sqrt30) / 35.0);
            td_[3][3] = std::sqrt((15.0 + 2.0 * sqrt30) / 35.0);

            // Weights for 4-point rule
            Ad_[1][3] = (1.0 / 3.0 * td_[0][3] * td_[0][3] - 0.2) /
                        (td_[1][3] * td_[1][3] * td_[0][3] * td_[0][3] -
                         td_[1][3] * td_[1][3] * td_[1][3] * td_[1][3]);
            Ad_[0][3] = (1.0 / 3.0 - Ad_[1][3] * td_[1][3] * td_[1][3]) / (td_[0][3] * td_[0][3]);
            Ad_[2][3] = Ad_[1][3];
            Ad_[3][3] = Ad_[0][3];
        }

        void PotentialFlowSolver::setup_derivative_coefficients()
        {
            // Finite difference coefficients for first derivatives
            // dst[i+2][j+2] for stencil point i at boundary type j
            // j = 0: left boundary (one-sided)
            // j = 1: near left boundary
            // j = 2: interior (centered)
            // j = 3: near right boundary
            // j = 4: right boundary (one-sided)

            // Left boundary (forward difference)
            dst_[0][0] = 3.0 / 12.0;
            dst_[1][0] = -16.0 / 12.0;
            dst_[2][0] = 36.0 / 12.0;
            dst_[3][0] = -48.0 / 12.0;
            dst_[4][0] = 25.0 / 12.0;

            // Near left boundary
            dst_[0][1] = -1.0 / 12.0;
            dst_[1][1] = 6.0 / 12.0;
            dst_[2][1] = -18.0 / 12.0;
            dst_[3][1] = 10.0 / 12.0;
            dst_[4][1] = 3.0 / 12.0;

            // Interior (centered)
            dst_[0][2] = 1.0 / 12.0;
            dst_[1][2] = -8.0 / 12.0;
            dst_[2][2] = 0.0;
            dst_[3][2] = 8.0 / 12.0;
            dst_[4][2] = -1.0 / 12.0;

            // Near right boundary
            dst_[0][3] = -3.0 / 12.0;
            dst_[1][3] = -10.0 / 12.0;
            dst_[2][3] = 18.0 / 12.0;
            dst_[3][3] = -6.0 / 12.0;
            dst_[4][3] = 1.0 / 12.0;

            // Right boundary (backward difference)
            dst_[0][4] = -25.0 / 12.0;
            dst_[1][4] = 48.0 / 12.0;
            dst_[2][4] = -36.0 / 12.0;
            dst_[3][4] = 16.0 / 12.0;
            dst_[4][4] = -3.0 / 12.0;
        }

        bool PotentialFlowSolver::setup_geometry(const io::AirfoilData &airfoil,
                                                 const ProjectConfig &config)
        {
            // Original code: DEFGEO
            if (airfoil.num_points < 10)
            {
                error_message_ = "Insufficient airfoil points";
                return false;
            }

            angle_of_attack_ = config.angle_of_attack * DEG_TO_RAD;
            int m_in = airfoil.num_points;

            // Copy and rotate coordinates around quarter chord
            RealVector y1in(m_in), y2in(m_in);
            Real cos_a = std::cos(angle_of_attack_);
            Real sin_a = std::sin(angle_of_attack_);

            for (int i = 0; i < m_in; ++i)
            {
                Real x = airfoil.x[i];
                Real y = airfoil.y[i];
                // Rotate around quarter chord point (0.25, 0)
                y1in[i] = (x - 0.25) * cos_a + y * sin_a + 0.25;
                y2in[i] = -(x - 0.25) * sin_a + y * cos_a;
            }

            // Create parameter for input coordinates
            RealVector ssin(m_in);
            for (int i = 0; i < m_in; ++i)
            {
                ssin[i] = static_cast<Real>(i);
            }

            // Spline the input coordinates
            RealVector d2y1in(m_in), d2y2in(m_in);
            spline_setup(ssin, y1in, m_in, d2y1in);
            spline_setup(ssin, y2in, m_in, d2y2in);

            // Set up panel distribution (cosine spacing)
            for (int i = 0; i <= n_; ++i)
            {
                // Real theta = PI * static_cast<Real>(i) / static_cast<Real>(n_);
                // swork_[i] = 0.5 * (1.0 - std::cos(theta)) * static_cast<Real>(n_);
                swork_[i] = static_cast<Real>(i);
            }

            // Compute panel lengths
            for (int i = 0; i < n_; ++i)
            {
                ds_[i] = swork_[i + 1] - swork_[i];
            }

            // Interpolate airfoil coordinates onto panel nodes
            for (int i = 0; i <= n_; ++i)
            {
                Real scp = swork_[i] * ssin[m_in - 1] / swork_[n_];
                int khi, klo;
                Real d1y1, d1y2;

                spline_search(ssin, m_in, scp, khi, klo);
                spline_interp(ssin, y1in, d2y1in, m_in, scp, yc1_[i], d1y1, khi, klo);
                spline_interp(ssin, y2in, d2y2in, m_in, scp, yc2_[i], d1y2, khi, klo);

                // Compute normals and tangents
                Real ds = std::sqrt(d1y1 * d1y1 + d1y2 * d1y2);
                nc1_[i] = d1y2 / ds;  // Normal x-component
                nc2_[i] = -d1y1 / ds; // Normal y-component
                tc1_[i] = d1y1 / ds;  // Tangent x-component
                tc2_[i] = d1y2 / ds;  // Tangent y-component
            }

            // Spline the panel node coordinates
            spline_setup(swork_, yc1_, n_ + 1, d2yc1_);
            spline_setup(swork_, yc2_, n_ + 1, d2yc2_);

            // Determine wake point at infinity //TODO: CHECK
            Real y1wh = 0.5 * (yc1_[1] + yc1_[n_]); // Wake half-point
            Real y2wh = 0.5 * (yc2_[1] + yc2_[n_]);

            Real ddss = std::sqrt((yc1_[0] - y1wh) * (yc1_[0] - y1wh) +
                                  (yc2_[0] - y2wh) * (yc2_[0] - y2wh));

            // Wake extends far downstream (100 chord lengths)
            ywinf1_ = yc1_[0] + (yc1_[0] - y1wh) / ddss * 100.0;
            ywinf2_ = yc2_[0] + (yc2_[0] - y2wh) / ddss * 100.0;

            // Wake normal direction
            ywn1_ = (yc2_[0] - y2wh) / ddss;
            ywn2_ = -(yc1_[0] - y1wh) / ddss;

            // !     Re-panel the airfoil contour for acoustics
            // Re-panel the airfoil contour for acoustics
            // na_ = number of acoustic panels (same as n_ by default)
            na_ = n_;

            sworkat_.resize(na_ + 1);
            yc1at_.resize(na_ + 1);
            yc2at_.resize(na_ + 1);
            nc1at_.resize(na_ + 1);
            nc2at_.resize(na_ + 1);
            tc1at_.resize(na_ + 1);
            tc2at_.resize(na_ + 1);
            d2yc1at_.resize(na_ + 1);
            d2yc2at_.resize(na_ + 1);

            // Set up acoustic panel arc length parameter
            for (int i = 0; i <= na_; ++i)
            {
                sworkat_[i] = static_cast<Real>(i);
            }

            // Interpolate flow panel coordinates onto acoustic panels
            for (int i = 0; i <= na_; ++i)
            {
                Real scp = sworkat_[i] * static_cast<Real>(n_) / static_cast<Real>(na_);
                int khi, klo;
                Real y1, y2, d1y1, d1y2;

                spline_search(swork_, n_ + 1, scp, khi, klo);
                spline_interp(swork_, yc1_, d2yc1_, n_ + 1, scp, y1, d1y1, khi, klo);
                spline_interp(swork_, yc2_, d2yc2_, n_ + 1, scp, y2, d1y2, khi, klo);

                yc1at_[i] = y1;
                yc2at_[i] = y2;
            }

            // Spline the acoustic panel coordinates (natural spline with large endpoint values)
            spline_setup_natural(sworkat_, yc1at_, na_ + 1, 1.0e33, 1.0e33, d2yc1at_);
            spline_setup_natural(sworkat_, yc2at_, na_ + 1, 1.0e33, 1.0e33, d2yc2at_);

            // Compute acoustic panel normals and tangents
            for (int i = 0; i <= na_; ++i)
            {
                Real scp = sworkat_[i] * static_cast<Real>(n_) / static_cast<Real>(na_);
                int khi, klo;
                Real y1, y2, d1y1, d1y2;

                spline_search(sworkat_, na_ + 1, scp, khi, klo);
                spline_interp(sworkat_, yc1at_, d2yc1at_, na_ + 1, scp, y1, d1y1, khi, klo);
                spline_interp(sworkat_, yc2at_, d2yc2at_, na_ + 1, scp, y2, d1y2, khi, klo);

                Real ds_mag = std::sqrt(d1y1 * d1y1 + d1y2 * d1y2);
                nc1at_[i] = d1y2 / ds_mag;
                nc2at_[i] = -d1y1 / ds_mag;
                tc1at_[i] = d1y1 / ds_mag;
                tc2at_[i] = d1y2 / ds_mag;
            }

            geometry_initialized_ = true;
            solution_computed_ = false;

            return true;
        }

        void PotentialFlowSolver::spline_setup_wrong_implemented(const RealVector &x, const RealVector &y,
                                                                 int n, RealVector &d2y)
        {
            // Original code: SPL_P - Natural spline
            // prepare spline interpolation
            d2y.resize(n, 0.0);
            RealVector u(n, 0.0);

            // Natural spline: second derivative = 0 at endpoints
            d2y[0] = 0.0;
            u[0] = 0.0;

            // Decomposition loop
            for (int i = 1; i < n - 1; ++i)
            {
                Real sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
                Real p = sig * d2y[i - 1] + 2.0;
                d2y[i] = (sig - 1.0) / p;
                u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
                u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
            }

            // Natural spline at right endpoint
            d2y[n - 1] = 0.0;

            // Back-substitution loop
            for (int k = n - 2; k >= 0; --k)
            {
                d2y[k] = d2y[k] * d2y[k + 1] + u[k];
            }
        }

        void PotentialFlowSolver::spline_setup(const RealVector &x, const RealVector &y, int n, RealVector &d2y)
        {
            // SPL_P - Cubic spline setup with computed endpoint derivatives
            // Uses 5-point finite difference to estimate endpoint slopes

            d2y.resize(n, 0.0);
            RealVector u(n, 0.0);

            // Compute endpoint derivatives using 5-point stencil
            Real yp1 = -25.0 / 12.0 * y[0] + 48.0 / 12.0 * y[1] - 36.0 / 12.0 * y[2] + 16.0 / 12.0 * y[3] - 3.0 / 12.0 * y[4];
            Real ypn = 3.0 / 12.0 * y[n - 5] - 16.0 / 12.0 * y[n - 4] + 36.0 / 12.0 * y[n - 3] - 48.0 / 12.0 * y[n - 2] + 25.0 / 12.0 * y[n - 1];

            // Lower boundary condition (clamped with computed slope)
            if (std::abs(yp1) > 0.99e30)
            {
                // Natural spline
                d2y[0] = 0.0;
                u[0] = 0.0;
            }
            else
            {
                // Clamped spline
                d2y[0] = -0.5;
                u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
            }

            // Decomposition loop
            for (int i = 1; i < n - 1; ++i)
            {
                Real sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
                Real p = sig * d2y[i - 1] + 2.0;
                d2y[i] = (sig - 1.0) / p;
                u[i] = (6.0 * ((y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1])) / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
            }

            // Upper boundary condition
            Real qn, un;
            if (std::abs(ypn) > 0.99e30)
            {
                // Natural spline
                qn = 0.0;
                un = 0.0;
            }
            else
            {
                // Clamped spline
                qn = 0.5;
                un = (3.0 / (x[n - 1] - x[n - 2])) * (ypn - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]));
            }

            d2y[n - 1] = (un - qn * u[n - 2]) / (qn * d2y[n - 2] + 1.0);

            // Backsubstitution loop
            for (int k = n - 2; k >= 0; --k)
            {
                d2y[k] = d2y[k] * d2y[k + 1] + u[k];
            }
        }

        void PotentialFlowSolver::spline_setup_natural(const RealVector &x, const RealVector &y,
                                                       int n, Real yp1, Real ypn, RealVector &d2y) const
        {
            // SPL_PP - Cubic spline setup with specified endpoint derivatives
            // If yp1 or ypn > 0.99e30, natural spline (y'' = 0) at that endpoint

            d2y.resize(n, 0.0);
            RealVector u(n, 0.0);

            // Lower boundary condition
            if (yp1 > 0.99e30)
            {
                // Natural spline at left endpoint
                d2y[0] = 0.0;
                u[0] = 0.0;
            }
            else
            {
                // Clamped spline with specified slope
                d2y[0] = -0.5;
                u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
            }

            // Decomposition loop
            for (int i = 1; i < n - 1; ++i)
            {
                Real sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
                Real p = sig * d2y[i - 1] + 2.0;
                d2y[i] = (sig - 1.0) / p;
                u[i] = (6.0 * ((y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1])) / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
            }

            // Upper boundary condition
            Real qn, un;
            if (ypn > 0.99e30)
            {
                // Natural spline at right endpoint
                qn = 0.0;
                un = 0.0;
            }
            else
            {
                // Clamped spline with specified slope
                qn = 0.5;
                un = (3.0 / (x[n - 1] - x[n - 2])) * (ypn - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]));
            }

            d2y[n - 1] = (un - qn * u[n - 2]) / (qn * d2y[n - 2] + 1.0);

            // Backsubstitution loop
            for (int k = n - 2; k >= 0; --k)
            {
                d2y[k] = d2y[k] * d2y[k + 1] + u[k];
            }
        }

        /**
         * @brief Binary search to find the bracketing interval for spline interpolation.
         *
         * Finds indices klo and khi such that xa[klo] <= x <= xa[khi],
         * where khi = klo + 1.
         *
         * @param x   Sorted array of knot positions (size n)
         * @param n    Number of elements in xa
         * @param xval    Value to locate
         * @param khi  Output: upper bracket index (0-based)
         * @param klo  Output: lower bracket index (0-based)
         */
        void PotentialFlowSolver::spline_search(const RealVector &x, int n, Real xval,
                                                int &khi, int &klo) const
        {
            // Original code: SPL_EX
            klo = 0;
            khi = n - 1;
            while (khi - klo > 1)
            {
                int k = (khi + klo) >> 1;
                if (x[k] > xval)
                {
                    khi = k;
                }
                else
                {
                    klo = k;
                }
            }
        }

        void PotentialFlowSolver::spline_interp(const RealVector &x, const RealVector &y,
                                                const RealVector &d2y, int /*n*/, Real xval,
                                                Real &yval, Real &dydx, int &khi, int &klo) const
        {
            // Original code: SPL_EX1
            //.             (swork_, pots_, d2pots_, n_ + 1,  s, ppp, tang,  khi, klo)
            // spline_interp(swork_, yc1_,  d2yc1_,  n_ + 1,  s, y1,  dy1ds, khi, klo);
            // SPL_EX1      (xa,     ya,    y2a,     n,       x, y,   dydx,  khi, klo)
            Real h = x[khi] - x[klo];
            if (h == 0.0)
            {
                yval = y[klo];
                dydx = 0.0;
                return;
            }

            Real a = (x[khi] - xval) / h;
            Real b = (xval - x[klo]) / h;

            // Spline interpolation
            // yval = a * y[klo] + b * y[khi] +
            //       ((a * a * a - a) * d2y[klo] + (b * b * b - b) * d2y[khi]) * (h * h) / 6.0;

            yval = a * y[klo] + b * y[khi] + ((a * a * a - a) * d2y[klo] + (b * b * b - b) * d2y[khi]) * (h * h) / 6.0;

            // First derivative
            // dydx = (y[khi] - y[klo]) / h -
            //       (3.0 * a * a - 1.0) / 6.0 * h * d2y[klo] +
            //       (3.0 * b * b - 1.0) / 6.0 * h * d2y[khi];

            dydx = (-y[klo] + y[khi]) / h + ((-3.0 * a * a + 1.0) * d2y[klo] + (+3.0 * b * b - 1.0) * d2y[khi]) * h / 6.0;
        }

        bool PotentialFlowSolver::solve()
        {
            if (!geometry_initialized_)
            {
                error_message_ = "Geometry not initialized";
                return false;
            }

            // Build influence coefficient matrix
            build_influence_matrix();

            // Set up right-hand side (boundary conditions)
            setup_rhs();

            // Solve the linear system
            if (!solve_system())
            {
                return false;
            }

            // Compute pressure distribution and lift coefficient
            compute_pressure_distribution();

            solution_computed_ = true;
            return true;
        }

        void PotentialFlowSolver::build_influence_matrix()
        {
            // SETMAT - Build the influence coefficient matrix for the panel method

            // Calculate solid angle at trailing edge
            // Original: rhelp = acos(-tc1(1)*tc1(n+1) - tc2(1)*tc2(n+1))
            Real rhelp = std::acos(-tc1_[0] * tc1_[n_] - tc2_[0] * tc2_[n_]);
            Real solidangle = 1.0 - 0.5 * (2.0 * rhelp / TWO_PI);

            // Initialize matrix to zero
            Kern_.setZero();

            // Build influence coefficients for each control point
            // Original: do i=1,N (1-based), C++: 0 to n_-1 (0-based)
            for (int i = 0; i < n_; ++i)
            {
                Real x1 = yc1_[i];
                Real x2 = yc2_[i];

                // Diagonal element (self-influence)
                // Original: Kern(i,i) = 0.5d0; if(i.eq.1) Kern(i,i) = solidangle
                Kern_(i, i) = 0.5;
                if (i == 0)
                {
                    Kern_(i, i) = solidangle;
                }

                // Influence from each panel
                for (int j = 0; j < n_; ++j)
                {
                    // Original: s1 = swork(j) + 0.0000001d0; s2 = swork(j+1) - 0.0000001d0
                    Real s1 = swork_[j] + 1.0e-7;
                    Real s2 = swork_[j + 1] - 1.0e-7;

                    Real herm1, herm2, herm3, herm4;
                    compute_panel_influence(x1, x2, s1, s2, herm1, herm2, herm3, herm4);

                    // Add contributions to influence matrix
                    // Original: do jshift=0,1
                    for (int jshift = 0; jshift <= 1; ++jshift)
                    {
                        // Original: Kern(i,j+jshift) = Kern(i,j+jshift) - HERM(i,j,jshift+1)
                        // HERM(i,j,1) = herm1, HERM(i,j,2) = herm2
                        Real herm_val = (jshift == 0) ? herm1 : herm2;
                        Kern_(i, j + jshift) -= herm_val;

                        // Derivative contributions using finite difference stencil
                        // Original ishift logic (converting from 1-based to 0-based):
                        // if(j+jshift.eq.1) ishift=2  -> if(j+jshift==0) ishift=2
                        // if(j+jshift.eq.2) ishift=1  -> if(j+jshift==1) ishift=1
                        // if(j+jshift.eq.n) ishift=-1 -> if(j+jshift==n_-1) ishift=-1
                        // if(j+jshift.eq.n+1) ishift=-2 -> if(j+jshift==n_) ishift=-2
                        int ishift = 0;
                        if (j + jshift == 0)
                            ishift = 2;
                        else if (j + jshift == 1)
                            ishift = 1;
                        else if (j + jshift == n_ - 1)
                            ishift = -1;
                        else if (j + jshift == n_)
                            ishift = -2;

                        // Original: HERM(i,j,jshift+3) means herm3 when jshift=0, herm4 when jshift=1
                        Real herm_deriv = (jshift == 0) ? herm3 : herm4;

                        // Original stencil application:
                        // Kern(i,j-2+ishift+jshift) -= dst(-2,ishift)*HERM(i,j,jshift+3)*.5
                        // Kern(i,j-1+ishift+jshift) -= dst(-1,ishift)*HERM(i,j,jshift+3)*.5
                        // Kern(i,j  +ishift+jshift) -= dst( 0,ishift)*HERM(i,j,jshift+3)*.5
                        // Kern(i,j+1+ishift+jshift) -= dst( 1,ishift)*HERM(i,j,jshift+3)*.5
                        // Kern(i,j+2+ishift+jshift) -= dst( 2,ishift)*HERM(i,j,jshift+3)*.5
                        //
                        // dst is indexed as dst(stencil_offset, boundary_type) where:
                        // stencil_offset: -2,-1,0,1,2 -> in C++ with offset: 0,1,2,3,4
                        // boundary_type: ishift (-2,-1,0,1,2) -> in C++ with offset: 0,1,2,3,4
                        for (int kk = -2; kk <= 2; ++kk)
                        {
                            int col_idx = j + kk + ishift + jshift;
                            if (col_idx >= 0 && col_idx <= n_)
                            {
                                // dst_[kk+2][ishift+2] maps (-2..2, -2..2) to (0..4, 0..4)
                                Kern_(i, col_idx) -= dst_[kk + 2][ishift + 2] * herm_deriv * 0.5;
                            }
                        }
                    }
                }

                // Wake contribution
                // Original: call calll(x1,x2,yc1(1),yc2(1),ywinf1,ywinf2,ywn1,ywn2,dipok,...)
                // Original: Kern(i,n+2) = -dipok
                Real dipok, v1, v2, dv1d1, dv1d2, dv2d1, dv2d2;
                compute_wake_contribution(x1, x2, yc1_[0], yc2_[0], ywinf1_, ywinf2_,
                                          ywn1_, ywn2_, dipok, v1, v2, dv1d1, dv1d2, dv2d1, dv2d2);
                Kern_(i, n_ + 1) = -dipok; // n+2 in 1-based = n_+1 in 0-based
            }

            // Kutta condition: velocity matching at trailing edge
            // Original uses the else branch (if(.false.) is never true):
            // Kern(n+1,1)   =  2.0    -> Kern_(n_, 0) = 2.0
            // Kern(n+1,2)   = -3.0    -> Kern_(n_, 1) = -3.0
            // Kern(n+1,3)   =  1.0    -> Kern_(n_, 2) = 1.0
            // Kern(n+1,n+1) = -2.0    -> Kern_(n_, n_) = -2.0
            // Kern(n+1,n)   =  3.0    -> Kern_(n_, n_-1) = 3.0
            // Kern(n+1,n-1) = -1.0    -> Kern_(n_, n_-2) = -1.0
            Kern_(n_, 0) = 2.0;
            Kern_(n_, 1) = -3.0;
            Kern_(n_, 2) = 1.0;
            Kern_(n_, n_) = -2.0;
            Kern_(n_, n_ - 1) = 3.0;
            Kern_(n_, n_ - 2) = -1.0;

            // Circulation equation
            // Original: Kern(n+2,1) = 1.0; Kern(n+2,n+1) = -1.0; Kern(n+2,n+2) = -1.0
            Kern_(n_ + 1, 0) = 1.0;
            Kern_(n_ + 1, n_) = -1.0;
            Kern_(n_ + 1, n_ + 1) = -1.0;

            // Modification for solid angle at TE
            // Original: Kern(1,n+2) = -solidangle/2.0
            Kern_(0, n_ + 1) = -solidangle / 2.0;

            // Perform LU factorization (like dgetrf in original)
            lu_solver_ = std::make_unique<Eigen::PartialPivLU<Eigen::MatrixXd>>(Kern_);
        }

        void PotentialFlowSolver::compute_panel_influence(Real x1, Real x2, Real s1, Real s2,
                                                          Real &herm1, Real &herm2,
                                                          Real &herm3, Real &herm4)
        {
            // CDI0 - Calculate panel influence using Gaussian quadrature
            // Uses Hermite interpolation for the potential distribution

            const Real pi2i = 1.0 / TWO_PI;

            herm1 = 0.0;
            herm2 = 0.0;
            herm3 = 0.0;
            herm4 = 0.0;

            // Midpoint of panel
            Real smid = (s1 + s2) / 2.0;
            int khi, klo;
            Real y1cent, y2cent, d1y1, d1y2;

            spline_search(swork_, n_ + 1, smid, khi, klo);
            spline_interp(swork_, yc1_, d2yc1_, n_ + 1, smid, y1cent, d1y1, khi, klo);
            spline_interp(swork_, yc2_, d2yc2_, n_ + 1, smid, y2cent, d1y2, khi, klo);

            // Check if we need adaptive integration (control point close to panel)
            // Original: raver = ((x1-y1cent)**2 + (x2-y2cent)**2) / ((s2-s1)**2)
            Real raver = ((x1 - y1cent) * (x1 - y1cent) + (x2 - y2cent) * (x2 - y2cent)) /
                         ((s2 - s1) * (s2 - s1));

            if (raver > 2.0)
            {
                // Standard Gaussian quadrature (far field)
                // Original: do k=0,ng-1; sloc = td(k,ng); s = (s1+s2)/2 + td(k,ng)*(s2-s1)/2; ...
                for (int k = 0; k < ng_; ++k)
                {
                    // Original: sloc = td(k, ng) - Fortran 1-based, so td[k][ng-1] in 0-based
                    Real sloc = td_[k][ng_ - 1];
                    Real s = (s1 + s2) / 2.0 + td_[k][ng_ - 1] * (s2 - s1) / 2.0;
                    Real wgtd = Ad_[k][ng_ - 1] * (s2 - s1) / 2.0;

                    Real y1, y2;
                    spline_search(swork_, n_ + 1, s, khi, klo);
                    spline_interp(swork_, yc1_, d2yc1_, n_ + 1, s, y1, d1y1, khi, klo);
                    spline_interp(swork_, yc2_, d2yc2_, n_ + 1, s, y2, d1y2, khi, klo);

                    // Normal vector components (outward normal)
                    // Original: n1 = d1y2; n2 = -d1y1
                    Real n1 = d1y2;
                    Real n2 = -d1y1;

                    // Distance squared
                    Real r2 = (x1 - y1) * (x1 - y1) + (x2 - y2) * (x2 - y2);

                    // Green's function derivative (dipole kernel)
                    // Original: green = pi2i * (n1*(x1-y1) + n2*(x2-y2)) / r2
                    Real green = pi2i * (n1 * (x1 - y1) + n2 * (x2 - y2)) / r2;

                    // Hermite basis functions - exact match to original
                    // Original formulas:
                    // herm1 = herm1 + wgtd*green*0.25*(2.0 - 3.0*sloc + sloc**3)
                    // herm2 = herm2 + wgtd*green*0.25*(2.0 + 3.0*sloc - sloc**3)
                    // herm3 = herm3 + wgtd*green*0.25*(1.0 - sloc - sloc**2 + sloc**3)
                    // herm4 = herm4 + wgtd*green*0.25*(-1.0 - sloc + sloc**2 + sloc**3)
                    Real sloc2 = sloc * sloc;
                    Real sloc3 = sloc2 * sloc;
                    herm1 += wgtd * green * 0.25 * (2.0 - 3.0 * sloc + sloc3);
                    herm2 += wgtd * green * 0.25 * (2.0 + 3.0 * sloc - sloc3);
                    herm3 += wgtd * green * 0.25 * (1.0 - sloc - sloc2 + sloc3);
                    herm4 += wgtd * green * 0.25 * (-1.0 - sloc + sloc2 + sloc3);
                }
            }
            else
            {
                // Near-singular case: use adaptive ODE integration
                // Original: odeint(ystart, nvar, s1, s2, eps, h1, hmin, nok, nbad, CDI0_f, rkqs)

                const Real eps = 1.0e-6;
                const Real h1 = 0.1;
                const Real hmin = 0.0;

                // Initial values (small non-zero to avoid issues)
                RealVector ystart = {1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6};
                int nvar = 4;
                int nok, nbad;

                // Create derivative function (CDI0_f) as a lambda
                // Captures x1, x2, s1, s2 and solver reference for spline access
                auto cdi0_f = [this, x1, x2, s1, s2, pi2i](Real s, const RealVector &y, RealVector &dydx)
                {
                    (void)y; // y is not used in CDI0_f

                    int khi_local, klo_local;
                    Real y1_local, y2_local, d1y1_local, d1y2_local;

                    spline_search(swork_, n_ + 1, s, khi_local, klo_local);
                    spline_interp(swork_, yc1_, d2yc1_, n_ + 1, s, y1_local, d1y1_local, khi_local, klo_local);
                    spline_interp(swork_, yc2_, d2yc2_, n_ + 1, s, y2_local, d1y2_local, khi_local, klo_local);

                    // Normal vector
                    Real n1 = d1y2_local;
                    Real n2 = -d1y1_local;

                    // Distance squared
                    Real r2 = (x1 - y1_local) * (x1 - y1_local) + (x2 - y2_local) * (x2 - y2_local);

                    // Local coordinate
                    Real sloc = (2.0 * s - s1 - s2) / (s2 - s1);

                    // Green's function
                    Real green = pi2i * (n1 * (x1 - y1_local) + n2 * (x2 - y2_local)) / r2;

                    // Hermite basis function derivatives (CDI0_f formulas)
                    Real sloc2 = sloc * sloc;
                    Real sloc3 = sloc2 * sloc;
                    dydx[0] = green * 0.25 * (2.0 - 3.0 * sloc + sloc3);
                    dydx[1] = green * 0.25 * (2.0 + 3.0 * sloc - sloc3);
                    dydx[2] = green * 0.25 * (1.0 - sloc - sloc2 + sloc3);
                    dydx[3] = green * 0.25 * (-1.0 - sloc + sloc2 + sloc3);
                };

                // Integrate using adaptive Runge-Kutta
                math::ODEIntegrator integrator;
                integrator.odeint(ystart, nvar, s1, s2, eps, h1, hmin, nok, nbad, cdi0_f);

                // Extract results (subtract initial small values)
                herm1 = ystart[0] - 1.0e-6;
                herm2 = ystart[1] - 1.0e-6;
                herm3 = ystart[2] - 1.0e-6;
                herm4 = ystart[3] - 1.0e-6;
            }
        }

        void PotentialFlowSolver::compute_wake_contribution(Real x1, Real x2, Real y1, Real y2,
                                                            Real /*ywinf1*/, Real /*ywinf2*/,
                                                            Real n1, Real n2,
                                                            Real &dipok, Real &v1, Real &v2,
                                                            Real &dv1d1, Real &dv1d2,
                                                            Real &dv2d1, Real &dv2d2)
        {
            // calll - Calculate wake contribution to potential and velocity
            const Real small = 1.0e-12;

            Real r1 = x1 - y1;
            Real r2 = x2 - y2;
            Real rsq = r1 * r1 + r2 * r2;
            Real z = r1 * n1 + r2 * n2;
            Real x = r1 * n2 - r2 * n1;
            Real thet1;

            // Calculate angle (handling all quadrants)
            if (z > small)
            {
                if (x < -small)
                    thet1 = std::atan(z / x) + PI;
                else if (std::abs(x) <= small)
                    thet1 = HALF_PI;
                else
                    thet1 = std::atan(z / x);
            }
            else if (std::abs(z) <= small)
            {
                thet1 = (x < -small) ? PI : 0.0;
            }
            else
            {
                if (x < -small)
                    thet1 = std::atan(z / x) - PI;
                else if (std::abs(x) <= small)
                    thet1 = -HALF_PI;
                else
                    thet1 = std::atan(z / x);
            }

            dipok = -thet1 / TWO_PI;

            // Velocity components in local coordinates
            Real v1_t = z / TWO_PI * (1.0 / rsq);
            Real v2_t = -1.0 / TWO_PI * (x / rsq);

            // Velocity derivatives
            Real dv1d1_t = -z * x / (PI * rsq * rsq);
            Real dv1d2_t = (rsq - 2.0 * z * z) / (TWO_PI * rsq * rsq);
            Real dv2d1_t = dv1d2_t;
            Real dv2d2_t = -dv1d1_t;

            // Transform to global coordinates
            v1 = n1 * v2_t + n2 * v1_t;
            v2 = n2 * v2_t - n1 * v1_t;
            dv1d1 = n2 * n2 * dv1d1_t + n1 * n2 * (dv1d2_t + dv2d1_t) + n1 * n1 * dv2d2_t;
            dv1d2 = n2 * n2 * dv1d2_t - n1 * n2 * (dv1d1_t - dv2d2_t) - n1 * n1 * dv2d1_t;
            dv2d1 = n2 * n2 * dv2d1_t - n1 * n2 * (dv1d1_t - dv2d2_t) - n1 * n1 * dv1d2_t;
            dv2d2 = n2 * n2 * dv2d2_t - n1 * n2 * (dv1d2_t + dv2d1_t) + n1 * n1 * dv1d1_t;
        }

        void PotentialFlowSolver::setup_rhs()
        {
            // SETRHS - Set up right-hand side (freestream boundary condition)
            // For potential formulation: phi = x (freestream in x-direction)

            rhs_.setZero();
            for (int i = 0; i < n_; ++i)
            {
                rhs_(i) = yc1_[i]; // Freestream potential = x-coordinate
            }
            rhs_(n_) = 0.0;     // Kutta condition
            rhs_(n_ + 1) = 0.0; // Circulation constraint
        }

        bool PotentialFlowSolver::solve_system()
        {
            // SOLSEQ - Solve the linear system using pre-factored LU decomposition
            // This matches the original: call dgetrs('N', n+2, 1, Kern, n+2, ipiv, rhs, n+2, info)

            if (!lu_solver_)
            {
                error_message_ = "LU factorization not performed";
                return false;
            }

            // Solve using pre-factored LU (like dgetrs)
            rhs_ = lu_solver_->solve(rhs_);

            // Extract surface potential distribution
            // Original: do i=1,n+1; pots(i) = rhs(i); enddo
            for (int i = 0; i <= n_; ++i)
            {
                pots_[i] = rhs_(i);
            }

            // Calculate endpoint derivatives (as in original, though not used by SPL_P)
            // yp1 = dst(-2,2)*rhs(1) + dst(-1,2)*rhs(2) + dst(0,2)*rhs(3) +
            //       dst(1,2)*rhs(4) + dst(2,2)*rhs(5)
            // ypn = dst(-2,-2)*rhs(n-3) + dst(-1,-2)*rhs(n-2) + dst(0,-2)*rhs(n-1) +
            //       dst(1,-2)*rhs(n) + dst(2,-2)*rhs(n+1)
            // Note: These are calculated but not used in the original code either

            // Spline the potential distribution
            // Original: call SPL_P(swork, pots, n+1, d2pots)
            spline_setup(swork_, pots_, n_ + 1, d2pots_);

            return true;
        }

        void PotentialFlowSolver::compute_pressure_distribution()
        {
            // DETCP - Compute pressure coefficient distribution and lift

            lift_coefficient_ = 0.0;

            for (int k = 0; k <= 1000; ++k)
            {
                Real s = swork_[n_] * static_cast<Real>(k) / 1000.0;
                int khi, klo;
                Real ppp, tang, y1, y2, dy1ds, dy2ds;

                spline_search(swork_, n_ + 1, s, khi, klo);
                spline_interp(swork_, pots_, d2pots_, n_ + 1, s, ppp, tang, khi, klo);
                spline_search(swork_, n_ + 1, s, khi, klo);
                spline_interp(swork_, yc1_, d2yc1_, n_ + 1, s, y1, dy1ds, khi, klo);
                spline_interp(swork_, yc2_, d2yc2_, n_ + 1, s, y2, dy2ds, khi, klo);

                // Velocity magnitude squared
                Real Usq = tang * tang / (dy1ds * dy1ds + dy2ds * dy2ds);

                // Pressure coefficient
                Real cp = 1.0 - Usq;

                // Integrate for lift
                lift_coefficient_ += cp * dy1ds * swork_[n_] / 1000.0;
            }

            std::cout << "  CL = " << lift_coefficient_ << std::endl;
        }

        Real PotentialFlowSolver::get_pressure_coefficient(Real s) const
        {
            if (!solution_computed_)
                return 0.0;

            int khi, klo;
            Real ppp, tang, y1, y2, dy1ds, dy2ds;

            spline_search(swork_, n_ + 1, s, khi, klo);
            spline_interp(swork_, pots_, d2pots_, n_ + 1, s, ppp, tang, khi, klo);
            spline_interp(swork_, yc1_, d2yc1_, n_ + 1, s, y1, dy1ds, khi, klo);
            spline_interp(swork_, yc2_, d2yc2_, n_ + 1, s, y2, dy2ds, khi, klo);

            Real Usq = tang * tang / (dy1ds * dy1ds + dy2ds * dy2ds);
            return 1.0 - Usq;
        }

        bool PotentialFlowSolver::calculate_streamlines(int num_streamlines, Real streamline_spacing,
                                                        io::StreamlineData &streamlines)
        {
            if (!solution_computed_)
            {
                error_message_ = "Solution not computed - call solve() first";
                return false;
            }

            compute_streamlines(num_streamlines, streamline_spacing, streamlines);
            return true;
        }

        void PotentialFlowSolver::compute_streamlines(int npath, Real dpath, io::StreamlineData &streamlines)
        {
            // DETSTR - Determine streamlines around the airfoil

            streamlines.resize(npath, nstr_);
            streamlines.streamline_spacing = dpath;
            streamlines.time_step = 0.001; // deltat

            // Initialize time array
            Real deltat = streamlines.time_step;
            for (int istr = 0; istr < nstr_; ++istr)
            {
                streamlines.time[istr] = static_cast<Real>(istr) * deltat;
            }

            // Find stagnation point
            Real tang_old = 0.0;
            Real s_stag = 0.0;

            for (int j = 1; j <= 10000; ++j)
            {
                Real s = static_cast<Real>(n_) * (0.25 + 0.5 * static_cast<Real>(j) / 10000.0);
                int khi, klo;
                Real ppp, tang;

                spline_search(swork_, n_ + 1, s, khi, klo);
                spline_interp(swork_, pots_, d2pots_, n_ + 1, s, ppp, tang, khi, klo);

                if (tang * tang_old < 0.0)
                {
                    s_stag = s;
                    break;
                }
                tang_old = tang;
            }

            // Get stagnation point coordinates
            int khi, klo;
            Real xstau1, xstau2, d1y1, d1y2;
            spline_search(swork_, n_ + 1, s_stag, khi, klo);
            spline_interp(swork_, yc1_, d2yc1_, n_ + 1, s_stag, xstau1, d1y1, khi, klo);
            spline_interp(swork_, yc2_, d2yc2_, n_ + 1, s_stag, xstau2, d1y2, khi, klo);

            streamlines.stagnation_x = xstau1;
            streamlines.stagnation_y = xstau2;

            // Trace back to find starting position upstream
            RealVector str1(nstr_), str2(nstr_), ps(nstr_);

            Real ds_mag = std::sqrt(d1y1 * d1y1 + d1y2 * d1y2);
            str1[0] = xstau1 + d1y2 * 0.005 / ds_mag; // Start slightly above surface
            str2[0] = xstau2 - d1y1 * 0.005 / ds_mag;

            integrate_streamline(-0.0025, str1, str2, ps);

            // Find where streamline crosses starting x-position
            Real xsta1 = -0.5; // Starting x upstream
            int ii = 0;
            for (int istr = 0; istr < nstr_ - 1; ++istr)
            {
                if ((str1[istr] - xsta1) * (str1[istr + 1] - xsta1) < 0.0)
                {
                    ii = istr;
                    break;
                }
            }
            Real xsta2 = str2[ii] + (str2[ii + 1] - str2[ii]) * (xsta1 - str1[ii]) / (str1[ii + 1] - str1[ii]);

            // Compute each streamline
            for (int ipath = 0; ipath < npath; ++ipath)
            {
                // Initial position with vertical offset
                Real y_offset = (static_cast<Real>(ipath) - static_cast<Real>(npath - 1) * 0.5) * dpath;

                streamlines.x[ipath][0] = xsta1;
                streamlines.y[ipath][0] = xsta2 + y_offset;

                // Integrate streamline forward
                RealVector path_x(nstr_), path_y(nstr_), path_pot(nstr_);
                path_x[0] = streamlines.x[ipath][0];
                path_y[0] = streamlines.y[ipath][0];

                integrate_streamline(deltat, path_x, path_y, path_pot);

                // Copy results
                for (int istr = 0; istr < nstr_; ++istr)
                {
                    streamlines.x[ipath][istr] = path_x[istr];
                    streamlines.y[ipath][istr] = path_y[istr];
                    streamlines.potential[ipath][istr] = path_pot[istr];
                }
            }

            //TODO: debug output: std::cout << "  Computed " << npath << " streamlines with " << nstr_ << " points each" << std::endl;
        }

        void PotentialFlowSolver::integrate_streamline(Real dt, RealVector &str1, RealVector &str2,
                                                       RealVector &ps)
        {
            // STREAM - Integrate streamline using velocity field from panel solution

            const Real pi2i = 1.0 / TWO_PI;

            for (int istr = 0; istr < nstr_ - 1; ++istr)
            {
                Real x1 = str1[istr];
                Real x2 = str2[istr];

                // Calculate velocity at current point by integrating over all panels
                Real phif0 = 0.0;                     // Potential
                Real v1 = 0.0, v2 = 0.0;              // Velocity components
                Real v11 = 0.0, v12 = 0.0, v22 = 0.0; // Velocity derivatives

                // Contribution from surface panels
                for (int j = 0; j < n_; ++j)
                {
                    Real s = (swork_[j] + swork_[j + 1]) / 2.0;
                    int khi, klo;
                    Real y1, y2, d1y1, d1y2, pot_val, dpot;

                    spline_search(swork_, n_ + 1, s, khi, klo);
                    spline_interp(swork_, yc1_, d2yc1_, n_ + 1, s, y1, d1y1, khi, klo);
                    spline_interp(swork_, yc2_, d2yc2_, n_ + 1, s, y2, d1y2, khi, klo);
                    spline_interp(swork_, pots_, d2pots_, n_ + 1, s, pot_val, dpot, khi, klo);

                    Real n1 = d1y2;
                    Real n2 = -d1y1;
                    Real d1 = x1 - y1;
                    Real d2 = x2 - y2;
                    Real r2 = d1 * d1 + d2 * d2;

                    if (r2 < 1e-10)
                        continue;

                    // Dipole contribution
                    Real ds_panel = swork_[j + 1] - swork_[j];
                    phif0 += (n1 * d1 + n2 * d2) / r2 * pot_val * pi2i * ds_panel;
                    v1 += (n1 * (d2 * d2 - d1 * d1) - 2.0 * n2 * d2 * d1) / (r2 * r2) * pot_val * pi2i * ds_panel;
                    v2 += (n2 * (d1 * d1 - d2 * d2) - 2.0 * n1 * d2 * d1) / (r2 * r2) * pot_val * pi2i * ds_panel;
                }

                // Wake contribution
                Real dipok, wv1, wv2, dv1d1, dv1d2, dv2d1, dv2d2;
                compute_wake_contribution(x1, x2, yc1_[0], yc2_[0], ywinf1_, ywinf2_,
                                          ywn1_, ywn2_, dipok, wv1, wv2, dv1d1, dv1d2, dv2d1, dv2d2);

                Real gamma = rhs_(n_ + 1); // Circulation
                phif0 += gamma * dipok;
                v1 += gamma * wv1;
                v2 += gamma * wv2;

                // Add freestream velocity
                v1 += 1.0; // Freestream in x-direction

                ps[istr] = phif0;

                // Advance streamline using Taylor expansion
                str1[istr + 1] = str1[istr] + v1 * dt + 0.5 * (v11 * v1 + v12 * v2) * dt * dt;
                str2[istr + 1] = str2[istr] + v2 * dt + 0.5 * (v12 * v1 + v22 * v2) * dt * dt;
            }

            // Set last potential value
            ps[nstr_ - 1] = ps[nstr_ - 2];
        }

    } // namespace potential
} // namespace bladenoise
