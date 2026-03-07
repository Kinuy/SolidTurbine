#pragma once

#include "bladenoise/core/Types.h"

namespace bladenoise {
namespace math {

class CubicSpline {
public:
    CubicSpline() = default;
    CubicSpline(const RealVector& x, const RealVector& y);

    void initialize(const RealVector& x, const RealVector& y, bool natural = true);
    void initialize_clamped(const RealVector& x, const RealVector& y,
                           Real yp1, Real ypn);

    Real evaluate(Real xi) const;
    Real derivative(Real xi) const;
    Real second_derivative(Real xi) const;
    Real inverse(Real xi, Real s_initial, Real tol = 1e-8) const;

    bool is_valid() const { return !x_.empty() && x_.size() == y_.size(); }

    const RealVector& x() const { return x_; }
    const RealVector& y() const { return y_; }

private:
    void solve_tridiagonal(const RealVector& a, const RealVector& b,
                          const RealVector& c, RealVector& d);
    size_t find_interval(Real xi) const;

    RealVector x_;
    RealVector y_;
    RealVector y2_;  // Second derivatives or first derivatives (depends on method)
};

class ParametricSpline2D {
public:
    ParametricSpline2D() = default;

    void initialize(const RealVector& x, const RealVector& y);

    Real eval_x(Real s) const;
    Real eval_y(Real s) const;
    Real eval_dxds(Real s) const;
    Real eval_dyds(Real s) const;
    Real curvature(Real s) const;

    const RealVector& arc_length() const { return s_; }
    Real total_length() const { return s_.empty() ? 0.0 : s_.back(); }

private:
    void calculate_arc_length(const RealVector& x, const RealVector& y);

    RealVector s_;
    CubicSpline x_spline_;
    CubicSpline y_spline_;
};

}  // namespace math
}  // namespace bladenoise
