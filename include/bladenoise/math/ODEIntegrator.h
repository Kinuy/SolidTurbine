#pragma once

#include "bladenoise/core/Types.h"
#include <functional>
#include <string>

namespace bladenoise {
namespace math {

class ODEIntegrator {
public:
    using DerivFunc = std::function<void(Real x, const RealVector& y, RealVector& dydx)>;

    bool odeint(RealVector& ystart, int nvar, Real x1, Real x2,
                Real eps, Real h1, Real hmin,
                int& nok, int& nbad,
                DerivFunc derivs);

    std::string get_error() const { return error_message_; }

private:
    bool rkqs(RealVector& y, const RealVector& dydx, int n,
              Real& x, Real htry, Real eps,
              const RealVector& yscal,
              Real& hdid, Real& hnext,
              DerivFunc derivs);

    void rkck(const RealVector& y, const RealVector& dydx, int n,
              Real x, Real h,
              RealVector& yout, RealVector& yerr,
              DerivFunc derivs);

    static constexpr int MAXSTP = 10000;
    static constexpr Real TINY = 1.0e-30;

    std::string error_message_;
};

}  // namespace math
}  // namespace bladenoise
