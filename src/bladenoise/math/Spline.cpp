#include "bladenoise/math/Spline.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace bladenoise {
namespace math {

CubicSpline::CubicSpline(const RealVector& x, const RealVector& y) {
    initialize(x, y);
}

void CubicSpline::initialize(const RealVector& x, const RealVector& y, bool natural) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("CubicSpline: x and y must have same size");
    }
    if (x.size() < 2) {
        throw std::invalid_argument("CubicSpline: need at least 2 points");
    }

    x_ = x;
    y_ = y;
    const size_t n = x_.size();
    y2_.resize(n, 0.0);

    if (natural) {
        // Natural spline: zero second derivative at endpoints
        RealVector a(n), b(n), c(n), d(n);

        // Interior points
        for (size_t i = 1; i < n - 1; ++i) {
            Real dsm = x_[i] - x_[i-1];
            Real dsp = x_[i+1] - x_[i];
            
            c[i] = dsp;          // Upper diagonal
            a[i] = 2.0 * (dsm + dsp);  // Main diagonal
            b[i] = dsm;          // Lower diagonal
            
            d[i] = 3.0 * ((y_[i+1] - y_[i]) * dsm / dsp + 
                          (y_[i] - y_[i-1]) * dsp / dsm);
        }

        // Natural boundary conditions (zero second derivative)
        a[0] = 2.0;
        c[0] = 1.0;
        d[0] = 3.0 * (y_[1] - y_[0]) / (x_[1] - x_[0]);

        b[n-1] = 1.0;
        a[n-1] = 2.0;
        d[n-1] = 3.0 * (y_[n-1] - y_[n-2]) / (x_[n-1] - x_[n-2]);

        solve_tridiagonal(a, b, c, d);
        y2_ = d;  // Store first derivatives
    }
}

void CubicSpline::initialize_clamped(const RealVector& x, const RealVector& y,
                                      Real yp1, Real ypn) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("CubicSpline: x and y must have same size");
    }
    if (x.size() < 2) {
        throw std::invalid_argument("CubicSpline: need at least 2 points");
    }

    x_ = x;
    y_ = y;
    const size_t n = x_.size();
    y2_.resize(n, 0.0);

    RealVector a(n), b(n), c(n), d(n);

    // Interior points
    for (size_t i = 1; i < n - 1; ++i) {
        Real dsm = x_[i] - x_[i-1];
        Real dsp = x_[i+1] - x_[i];
        
        c[i] = dsp;
        a[i] = 2.0 * (dsm + dsp);
        b[i] = dsm;
        
        d[i] = 3.0 * ((y_[i+1] - y_[i]) * dsm / dsp + 
                      (y_[i] - y_[i-1]) * dsp / dsm);
    }

    // Clamped boundary conditions
    a[0] = 1.0;
    c[0] = 0.0;
    d[0] = yp1;

    b[n-1] = 0.0;
    a[n-1] = 1.0;
    d[n-1] = ypn;

    solve_tridiagonal(a, b, c, d);
    y2_ = d;
}

void CubicSpline::solve_tridiagonal(const RealVector& a, const RealVector& b,
                                     const RealVector& c, RealVector& d) {
    const size_t n = a.size();
    RealVector cp(n), dp(n);

    // Forward sweep
    cp[0] = c[0] / a[0];
    dp[0] = d[0] / a[0];

    for (size_t i = 1; i < n; ++i) {
        Real m = a[i] - b[i] * cp[i-1];
        cp[i] = c[i] / m;
        dp[i] = (d[i] - b[i] * dp[i-1]) / m;
    }

    // Back substitution
    d[n-1] = dp[n-1];
    for (size_t i = n - 1; i > 0; --i) {
        d[i-1] = dp[i-1] - cp[i-1] * d[i];
    }
}

size_t CubicSpline::find_interval(Real xi) const {
    // Binary search for interval
    size_t lo = 0;
    size_t hi = x_.size() - 1;

    while (hi - lo > 1) {
        size_t mid = (hi + lo) / 2;
        if (xi < x_[mid]) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    return lo;
}

Real CubicSpline::evaluate(Real xi) const {
    if (!is_valid()) {
        throw std::runtime_error("CubicSpline: not initialized");
    }

    size_t i = find_interval(xi);
    
    Real ds = x_[i+1] - x_[i];
    Real t = (xi - x_[i]) / ds;
    
    Real cx1 = ds * y2_[i] - y_[i+1] + y_[i];
    Real cx2 = ds * y2_[i+1] - y_[i+1] + y_[i];
    
    return t * y_[i+1] + (1.0 - t) * y_[i] + 
           (t - t*t) * ((1.0 - t) * cx1 - t * cx2);
}

Real CubicSpline::derivative(Real xi) const {
    if (!is_valid()) {
        throw std::runtime_error("CubicSpline: not initialized");
    }

    size_t i = find_interval(xi);
    
    Real ds = x_[i+1] - x_[i];
    Real t = (xi - x_[i]) / ds;
    
    Real cx1 = ds * y2_[i] - y_[i+1] + y_[i];
    Real cx2 = ds * y2_[i+1] - y_[i+1] + y_[i];
    
    Real dydx = (y_[i+1] - y_[i] + 
                 (1.0 - 4.0*t + 3.0*t*t) * cx1 + 
                 t * (3.0*t - 2.0) * cx2) / ds;
    
    return dydx;
}

Real CubicSpline::second_derivative(Real xi) const {
    if (!is_valid()) {
        throw std::runtime_error("CubicSpline: not initialized");
    }

    size_t i = find_interval(xi);
    
    Real ds = x_[i+1] - x_[i];
    Real t = (xi - x_[i]) / ds;
    
    Real cx1 = ds * y2_[i] - y_[i+1] + y_[i];
    Real cx2 = ds * y2_[i+1] - y_[i+1] + y_[i];
    
    Real d2ydx2 = ((6.0*t - 4.0) * cx1 + (6.0*t - 2.0) * cx2) / (ds * ds);
    
    return d2ydx2;
}

Real CubicSpline::inverse(Real xi, Real s_initial, Real tol) const {
    Real s = s_initial;
    
    for (int iter = 0; iter < 10; ++iter) {
        Real res = evaluate(s) - xi;
        Real resp = derivative(s);
        Real ds = -res / resp;
        s += ds;
        
        if (std::abs(ds / (x_.back() - x_.front())) < tol) {
            return s;
        }
    }
    
    return s_initial;  // Return initial guess if convergence failed
}

// ParametricSpline2D implementation

void ParametricSpline2D::calculate_arc_length(const RealVector& x, const RealVector& y) {
    const size_t n = x.size();
    s_.resize(n);
    s_[0] = 0.0;
    
    for (size_t i = 1; i < n; ++i) {
        Real dx = x[i] - x[i-1];
        Real dy = y[i] - y[i-1];
        s_[i] = s_[i-1] + std::sqrt(dx*dx + dy*dy);
    }
}

void ParametricSpline2D::initialize(const RealVector& x, const RealVector& y) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("ParametricSpline2D: x and y must have same size");
    }
    if (x.size() < 2) {
        throw std::invalid_argument("ParametricSpline2D: need at least 2 points");
    }

    // Calculate arc length parametrization
    calculate_arc_length(x, y);

    // Initialize splines using arc length as parameter
    x_spline_.initialize(s_, x);
    y_spline_.initialize(s_, y);
}

Real ParametricSpline2D::eval_x(Real s) const {
    return x_spline_.evaluate(s);
}

Real ParametricSpline2D::eval_y(Real s) const {
    return y_spline_.evaluate(s);
}

Real ParametricSpline2D::eval_dxds(Real s) const {
    return x_spline_.derivative(s);
}

Real ParametricSpline2D::eval_dyds(Real s) const {
    return y_spline_.derivative(s);
}

Real ParametricSpline2D::curvature(Real s) const {
    Real xd = x_spline_.derivative(s);
    Real yd = y_spline_.derivative(s);
    Real xdd = x_spline_.second_derivative(s);
    Real ydd = y_spline_.second_derivative(s);
    
    Real sd = std::sqrt(xd*xd + yd*yd);
    sd = std::max(sd, 1e-10);  // Avoid division by zero
    
    return (xd * ydd - yd * xdd) / (sd * sd * sd);
}

}  // namespace math
}  // namespace bladenoise
