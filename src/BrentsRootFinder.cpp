/**
 * @file BrentsRootFinder.cpp
 * @brief Implementation of BrentsRootFinder.
 *
 * Brent's method is entirely self-contained; the header holds the full
 * algorithm.  This file exists for:
 *   1. Explicit instantiation if a translation unit needs it.
 *   2. A home for any future Brent-related utilities (e.g. bracketing helpers).
 */
#include "BrentsRootFinder.h"

// The full algorithm is in the header (template/inline).
// No additional definitions are required.

// ─────────────────────────────────────────────────────────────────────────────
// BracketRoot — utility: scan [x1, x2] with n subintervals and return the
// first bracket [a, b] where f changes sign.
//
// Use this before calling Solve() when you do not know a suitable bracket.
// ─────────────────────────────────────────────────────────────────────────────
std::optional<std::pair<double, double>>
BracketRoot(ScalarFn const &f, double x1, double x2, unsigned n)
{
    double dx = (x2 - x1) / static_cast<double>(n);
    double x = x1;
    double fp = f(x);
    for (unsigned i = 0; i < n; ++i)
    {
        x += dx;
        double fc = f(x);
        if (fp * fc <= 0.0)
            return std::make_pair(x - dx, x);
        fp = fc;
    }
    return std::nullopt;
}
