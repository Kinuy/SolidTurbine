#pragma once
/**
 * @file BrentsRootFinder.h
 * @brief Brent's method implementation of IRootFinder.
 *
 * Single Responsibility: owns only the root-finding algorithm.
 * No aerodynamic knowledge lives here.
 */
#include <cmath>
#include <optional>
#include <stdexcept>
#include "IRootFinder.h"

class BrentsRootFinder final : public IRootFinder
{
public:
    explicit BrentsRootFinder(double tolerance = 1e-6,
                              unsigned max_iters = 400)
        : tol_(tolerance), max_iters_(max_iters) {}

    std::optional<double> Solve(ScalarFn f,
                                double lower,
                                double upper) const override
    {
        double a = lower, b = upper;
        double fa = f(a), fb = f(b);

        if (fa * fb >= 0.0)
        {
            return std::nullopt; // caller must bracket correctly
        }

        // Ensure |f(a)| >= |f(b)|
        if (std::abs(fa) < std::abs(fb))
        {
            std::swap(a, b);
            std::swap(fa, fb);
        }

        double c = a, fc = fa;
        bool mflag = true;
        double s = 0.0, d = 0.0;

        for (unsigned it = 0; it < max_iters_; ++it)
        {
            if (std::abs(b - a) < tol_)
            {
                return s;
            }

            if (fa != fc && fb != fc)
            {
                // Inverse quadratic interpolation
                s = a * fb * fc / ((fa - fb) * (fa - fc)) + b * fa * fc / ((fb - fa) * (fb - fc)) + c * fa * fb / ((fc - fa) * (fc - fb));
            }
            else
            {
                // Secant method
                s = b - fb * (b - a) / (fb - fa);
            }

            bool cond1 = !((3.0 * a + b) / 4.0 < s && s < b) || !((b - a) / 4.0 > s - b && s - b > (a - b) / 4.0);
            bool cond2 = mflag && std::abs(s - b) >= std::abs(b - c) / 2.0;
            bool cond3 = !mflag && std::abs(s - b) >= std::abs(c - d) / 2.0;
            bool cond4 = mflag && std::abs(b - c) < tol_;
            bool cond5 = !mflag && std::abs(c - d) < tol_;

            if (cond1 || cond2 || cond3 || cond4 || cond5)
            {
                s = (a + b) / 2.0;
                mflag = true;
            }
            else
            {
                mflag = false;
            }

            double fs = f(s);
            d = c;
            c = b;
            fc = fb;

            if (fa * fs < 0.0)
            {
                b = s;
                fb = fs;
            }
            else
            {
                a = s;
                fa = fs;
            }

            if (std::abs(fa) < std::abs(fb))
            {
                std::swap(a, b);
                std::swap(fa, fb);
            }
        }
        return std::nullopt; // did not converge
    }

private:
    double tol_;
    unsigned max_iters_;
};
