#include "bladenoise/math/SpecialFunctions.h"
#include "bladenoise/core/Constants.h"
#include <cmath>
#include <algorithm>

namespace bladenoise {
namespace math {

// Polynomial coefficients for Bessel function approximations
namespace detail {
    // For J0, x < 8
    constexpr double J0_P[] = {
        1.0, -0.1098628627e-2, 0.2734510407e-4, -0.2073370639e-5,
        0.2093887211e-6
    };
    constexpr double J0_Q[] = {
        -0.1562499995e-1, 0.1430488765e-3, -0.6911147651e-5,
        0.7621095161e-6, -0.934945152e-7
    };
    
    // For J1, x < 8
    constexpr double J1_P[] = {
        0.5, -0.56249985e-1, 0.21093573e-2, -0.39545e-3,
        0.3520e-4
    };
    constexpr double J1_Q[] = {
        -0.4687499995e-1, 0.2002690873e-3, -0.8449199096e-5,
        0.88228987e-6, -0.105787412e-6
    };
}

Real SpecialFunctions::besselJ0(Real x) {
    Real ax = std::abs(x);
    
    if (ax < 8.0) {
        Real y = x * x;
        Real ans1 = 57568490574.0 + y * (-13362590354.0 + y * (651619640.7
            + y * (-11214424.18 + y * (77392.33017 + y * (-184.9052456)))));
        Real ans2 = 57568490411.0 + y * (1029532985.0 + y * (9494680.718
            + y * (59272.64853 + y * (267.8532712 + y * 1.0))));
        return ans1 / ans2;
    }
    
    Real z = 8.0 / ax;
    Real y = z * z;
    Real xx = ax - 0.785398164;
    
    Real ans1 = 1.0 + y * (-0.1098628627e-2 + y * (0.2734510407e-4
        + y * (-0.2073370639e-5 + y * 0.2093887211e-6)));
    Real ans2 = -0.1562499995e-1 + y * (0.1430488765e-3
        + y * (-0.6911147651e-5 + y * (0.7621095161e-6
        - y * 0.934945152e-7)));
    
    return std::sqrt(0.636619772 / ax) * (std::cos(xx) * ans1 - z * std::sin(xx) * ans2);
}

Real SpecialFunctions::besselJ1(Real x) {
    Real ax = std::abs(x);
    
    if (ax < 8.0) {
        Real y = x * x;
        Real ans1 = x * (72362614232.0 + y * (-7895059235.0 + y * (242396853.1
            + y * (-2972611.439 + y * (15704.48260 + y * (-30.16036606))))));
        Real ans2 = 144725228442.0 + y * (2300535178.0 + y * (18583304.74
            + y * (99447.43394 + y * (376.9991397 + y * 1.0))));
        return ans1 / ans2;
    }
    
    Real z = 8.0 / ax;
    Real y = z * z;
    Real xx = ax - 2.356194491;
    
    Real ans1 = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4
        + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
    Real ans2 = 0.04687499995 + y * (-0.2002690873e-3
        + y * (0.8449199096e-5 + y * (-0.88228987e-6
        + y * 0.105787412e-6)));
    
    Real ans = std::sqrt(0.636619772 / ax) * (std::cos(xx) * ans1 - z * std::sin(xx) * ans2);
    return x < 0.0 ? -ans : ans;
}

Real SpecialFunctions::besselY0(Real x) {
    if (x < 8.0) {
        Real y = x * x;
        Real ans1 = -2957821389.0 + y * (7062834065.0 + y * (-512359803.6
            + y * (10879881.29 + y * (-86327.92757 + y * 228.4622733))));
        Real ans2 = 40076544269.0 + y * (745249964.8 + y * (7189466.438
            + y * (47447.26470 + y * (226.1030244 + y * 1.0))));
        return (ans1 / ans2) + 0.636619772 * besselJ0(x) * std::log(x);
    }
    
    Real z = 8.0 / x;
    Real y = z * z;
    Real xx = x - 0.785398164;
    
    Real ans1 = 1.0 + y * (-0.1098628627e-2 + y * (0.2734510407e-4
        + y * (-0.2073370639e-5 + y * 0.2093887211e-6)));
    Real ans2 = -0.1562499995e-1 + y * (0.1430488765e-3
        + y * (-0.6911147651e-5 + y * (0.7621095161e-6
        - y * 0.934945152e-7)));
    
    return std::sqrt(0.636619772 / x) * (std::sin(xx) * ans1 + z * std::cos(xx) * ans2);
}

Real SpecialFunctions::besselY1(Real x) {
    if (x < 8.0) {
        Real y = x * x;
        Real ans1 = x * (-4900604943000.0 + y * (1275274390000.0
            + y * (-51534381390.0 + y * (734926455.1
            + y * (-4237922.726 + y * 8511.937935)))));
        Real ans2 = 24909857930000.0 + y * (424441966400.0
            + y * (3733650367.0 + y * (22459040.02
            + y * (102042.605 + y * (354.9632885 + y)))));
        return (ans1 / ans2) + 0.636619772 * (besselJ1(x) * std::log(x) - 1.0 / x);
    }
    
    Real z = 8.0 / x;
    Real y = z * z;
    Real xx = x - 2.356194491;
    
    Real ans1 = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4
        + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
    Real ans2 = 0.04687499995 + y * (-0.2002690873e-3
        + y * (0.8449199096e-5 + y * (-0.88228987e-6
        + y * 0.105787412e-6)));
    
    return std::sqrt(0.636619772 / x) * (std::sin(xx) * ans1 + z * std::cos(xx) * ans2);
}

Complex SpecialFunctions::hankel1_0(Real x) {
    return Complex(besselJ0(x), besselY0(x));
}

Complex SpecialFunctions::hankel1_1(Real x) {
    return Complex(besselJ1(x), besselY1(x));
}

// MathUtils implementation

Real MathUtils::safe_log10(Real x, Real min_val) {
    if (x <= 0.0) return min_val;
    return std::log10(x);
}

Real MathUtils::to_dB(Real pressure_ratio) {
    return 10.0 * safe_log10(pressure_ratio);
}

Real MathUtils::from_dB(Real dB) {
    return std::pow(10.0, dB / 10.0);
}

Real MathUtils::compute_OASPL(const RealVector& spl) {
    Real total_pressure = 0.0;
    for (Real s : spl) {
        if (s > -99.0) {  // Only skip floor values (-100 dB)
            total_pressure += from_dB(s);
        }
    }
    if (total_pressure <= 0.0) return -100.0;
    return to_dB(total_pressure);
}

Real MathUtils::sign(Real x) {
    if (x > 0.0) return 1.0;
    if (x < 0.0) return -1.0;
    return 0.0;
}

Real MathUtils::clamp(Real x, Real min_val, Real max_val) {
    return std::max(min_val, std::min(max_val, x));
}

}  // namespace math
}  // namespace bladenoise
