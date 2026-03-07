#pragma once

#include "bladenoise/core/Types.h"

namespace bladenoise {
namespace math {

class SpecialFunctions {
public:
    static Real besselJ0(Real x);
    static Real besselJ1(Real x);
    static Real besselY0(Real x);
    static Real besselY1(Real x);

    static Complex hankel1_0(Real x);
    static Complex hankel1_1(Real x);
};

class MathUtils {
public:
    static Real safe_log10(Real x, Real min_val = -100.0);
    static Real to_dB(Real pressure_ratio);
    static Real from_dB(Real dB);
    static Real compute_OASPL(const RealVector& spl);
    static Real sign(Real x);
    static Real clamp(Real x, Real min_val, Real max_val);
};

}  // namespace math
}  // namespace bladenoise
