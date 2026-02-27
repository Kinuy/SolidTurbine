#include "mathutilities.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cfloat>
#include <iostream>
#include <sstream>

namespace WVPMUtilities {

// ---------------------------------------------------------------------------
// Matrix operations
// ---------------------------------------------------------------------------

SquareMatrix<double> MatMult(SquareMatrix<double> const& m1,
                             SquareMatrix<double> const& m2)
{
    const int dim = m1.dim();
    SquareMatrix<double> result(dim);

    for (int row = 0; row < dim; ++row) {
        for (int col = 0; col < dim; ++col) {
            double sum = 0.0;
            for (int k = 0; k < dim; ++k) {
                sum += m1[row * dim + k] * m2[k * dim + col];
            }
            result[row * dim + col] = sum;
        }
    }
    return result;
}

void RotateVec3D(SquareMatrix<double> const& rotation_matrix,
                 Vec3D<double>* target_vec)
{
    // Pack the vector into the third column of a 3x3 matrix,
    // multiply, then unpack the result.
    SquareMatrix<double> col_matrix(3);
    col_matrix[2] = target_vec->x();
    col_matrix[5] = target_vec->y();
    col_matrix[8] = target_vec->z();

    const SquareMatrix<double> product = MatMult(rotation_matrix, col_matrix);

    (*target_vec)[0] = product[2];
    (*target_vec)[1] = product[5];
    (*target_vec)[2] = product[8];
}

// ---------------------------------------------------------------------------
// Interpolation helpers
// ---------------------------------------------------------------------------

std::map<double, double> fill_for_linear_interpolation(
    std::vector<double> const& x,
    std::vector<double> const& y)
{
    std::map<double, double> table;
    for (std::size_t i = 0; i < x.size(); ++i) {
        table.emplace(x[i], y[i]);
    }
    return table;
}

double linear_interpolation(double x,
                            std::map<double, double> const& table)
{
    if (table.empty()) {
        return 0.0;
    }

    auto it = table.lower_bound(x);

    if (it == table.end()) {
        return table.rbegin()->second;   // clamp to upper bound
    }
    if (it == table.begin()) {
        return it->second;               // clamp to lower bound
    }

    const double x2 = it->first;
    const double y2 = it->second;
    --it;
    const double x1 = it->first;
    const double y1 = it->second;

    const double t = (x - x1) / (x2 - x1);
    return (1.0 - t) * y1 + t * y2;
}

// Stub — replace with a real cubic spline when available.
double SplineForY(double /*target_x*/,
                  std::vector<double> const& /*x*/,
                  std::vector<double> const& /*y*/)
{
    return 1.0;
}

// ---------------------------------------------------------------------------
// Root finding — Brent's method
// ---------------------------------------------------------------------------

double brents_fun(std::function<double(double)> f,
                  double lower_bound,
                  double upper_bound)
{
    constexpr double kTol      = 1.0e-4;
    constexpr int    kMaxIter  = 1000;

    double a  = lower_bound,  b  = upper_bound;
    double fa = f(a),         fb = f(b);

    if (fa * fb >= 0.0) {
        std::cout << "[brents_fun] f(lower) and f(upper) must have opposite signs.\n";
        return 0.0;
    }

    if (std::abs(fa) < std::abs(fb)) {
        std::swap(a, b);
        std::swap(fa, fb);
    }

    double c = a, fc = fa;
    double s = 0.0, d = 0.0;
    bool mflag = true;

    for (int iter = 0; iter < kMaxIter; ++iter) {
        if (std::abs(b - a) < kTol) {
            return s;
        }

        if (fa != fc && fb != fc) {
            // Inverse quadratic interpolation
            s = (a * fb * fc) / ((fa - fb) * (fa - fc))
              + (b * fa * fc) / ((fb - fa) * (fb - fc))
              + (c * fa * fb) / ((fc - fa) * (fc - fb));
        } else {
            // Secant method
            s = b - fb * (b - a) / (fb - fa);
        }

        const bool cond1 = (s < (3.0 * a + b) * 0.25) || (s > b);
        const bool cond2 =  mflag && std::abs(s - b) >= std::abs(b - c) * 0.5;
        const bool cond3 = !mflag && std::abs(s - b) >= std::abs(c - d) * 0.5;
        const bool cond4 =  mflag && std::abs(b - c) < kTol;
        const bool cond5 = !mflag && std::abs(c - d) < kTol;

        if (cond1 || cond2 || cond3 || cond4 || cond5) {
            s = (a + b) * 0.5;   // bisection fallback
            mflag = true;
        } else {
            mflag = false;
        }

        const double fs = f(s);
        d  = c;
        c  = b;
        fc = fb;

        if (fa * fs < 0.0) {
            b  = s;
            fb = fs;
        } else {
            a  = s;
            fa = fs;
        }

        if (std::abs(fa) < std::abs(fb)) {
            std::swap(a, b);
            std::swap(fa, fb);
        }
    }

    std::cout << "[brents_fun] Did not converge within " << kMaxIter << " iterations.\n";
    return s;
}

// ---------------------------------------------------------------------------
// Special functions — Gamma / LogGamma
// ---------------------------------------------------------------------------

double LogGamma(double x)
{
    if (x <= 0.0) {
        std::ostringstream oss;
        oss << "LogGamma: invalid argument " << x << " (must be > 0).";
        throw std::invalid_argument(oss.str());
    }

    if (x < 12.0) {
        return std::log(std::abs(Gamma(x)));
    }

    // Asymptotic series — Abramowitz & Stegun 6.1.41
    static const double c[8] = {
         1.0 / 12.0,
        -1.0 / 360.0,
         1.0 / 1260.0,
        -1.0 / 1680.0,
         1.0 / 1188.0,
        -691.0 / 360360.0,
         1.0 / 156.0,
        -3617.0 / 122400.0
    };

    const double z = 1.0 / (x * x);
    double series = c[7];
    for (int i = 6; i >= 0; --i) {
        series = series * z + c[i];
    }
    series /= x;

    static constexpr double kHalfLogTwoPi = 0.91893853320467274178;
    return (x - 0.5) * std::log(x) - x + kHalfLogTwoPi + series;
}

double Gamma(double x)
{
    if (x <= 0.0 && x == std::floor(x)) {
        // Poles at non-positive integers
        return DBL_MAX * 2.0;
    }

    // Reduce to interval (1, 2) via recurrence
    double y = x;
    int    n = 0;
    bool   arg_was_less_than_one = (y < 1.0);

    if (arg_was_less_than_one) {
        y += 1.0;
    } else {
        while (y >= 2.0) {
            --n;
            y -= 1.0;
        }
    }

    // Rational minimax approximation on (1, 2)
    // (Hart, "Computer Approximations", Table 5.47)
    static const double p[8] = {
        -1.71618513886549492533811E+0,
         2.47656508055759199108314E+1,
        -3.79804256470945635097577E+2,
         6.29331155312818442661052E+2,
         8.66966202790413211295064E+2,
        -3.14512729688483675254357E+4,
        -3.61444134186911729807069E+4,
         6.64561438202405440627855E+4
    };
    static const double q[8] = {
        -3.08402300119738975254353E+1,
         3.15350626979604161529144E+2,
        -1.01515636749021914166146E+3,
        -3.10777167157231109440444E+3,
         2.25381184209801510330112E+4,
         4.75584627752788110767815E+3,
        -1.34659959864969306392456E+5,
        -1.15132259675553483497211E+5
    };

    const double z = y - 1.0;
    double num = 0.0, den = 1.0;
    for (int i = 0; i < 8; ++i) {
        num = (num + p[i]) * z;
        den =  den * z + q[i];
    }
    double result = num / den + 1.0;

    if (arg_was_less_than_one) {
        result /= (y - 1.0);
    } else {
        for (int i = 0; i > n; --i) {
            result *= y++;
        }
    }

    if (x > 171.624) {
        return DBL_MAX * 2.0;   // overflow to +infinity
    }

    return (x >= 12.0) ? std::exp(LogGamma(x)) : result;
}

} // namespace WVPMUtilities
