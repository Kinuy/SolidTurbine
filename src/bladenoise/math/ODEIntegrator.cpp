#include "bladenoise/math/ODEIntegrator.h"
#include <cmath>
#include <algorithm>

namespace bladenoise
{
    namespace math
    {

        bool ODEIntegrator::odeint(RealVector &ystart, int nvar, Real x1, Real x2,
                                   Real eps, Real h1, Real hmin,
                                   int &nok, int &nbad,
                                   DerivFunc derivs)
        {
            // ODE integrator with adaptive step size control
            // Based on Numerical Recipes odeint routine

            Real x = x1;
            Real h = std::copysign(h1, x2 - x1);
            nok = 0;
            nbad = 0;

            RealVector y(nvar);
            RealVector dydx(nvar);
            RealVector yscal(nvar);

            // Copy initial conditions
            for (int i = 0; i < nvar; ++i)
            {
                y[i] = ystart[i];
            }

            // Main integration loop
            for (int nstp = 0; nstp < MAXSTP; ++nstp)
            {
                // Compute derivatives
                derivs(x, y, dydx);

                // Scaling for error estimation
                for (int i = 0; i < nvar; ++i)
                {
                    yscal[i] = std::abs(y[i]) + std::abs(h * dydx[i]) + TINY;
                }

                // Adjust step size if we would overshoot
                if ((x + h - x2) * (x + h - x1) > 0.0)
                {
                    h = x2 - x;
                }

                // Take a quality-controlled step
                Real hdid, hnext;
                if (!rkqs(y, dydx, nvar, x, h, eps, yscal, hdid, hnext, derivs))
                {
                    return false; // Step failed
                }

                if (hdid == h)
                {
                    ++nok;
                }
                else
                {
                    ++nbad;
                }

                // Check if we've reached the end
                if ((x - x2) * (x2 - x1) >= 0.0)
                {
                    // Copy final values back
                    for (int i = 0; i < nvar; ++i)
                    {
                        ystart[i] = y[i];
                    }
                    return true;
                }

                // Check minimum step size
                if (std::abs(hnext) < hmin)
                {
                    error_message_ = "Step size smaller than minimum in odeint";
                    // Continue anyway, just warn
                }

                h = hnext;
            }

            error_message_ = "Too many steps in odeint";
            return false;
        }

        bool ODEIntegrator::rkqs(RealVector &y, const RealVector &dydx, int n,
                                 Real &x, Real htry, Real eps,
                                 const RealVector &yscal,
                                 Real &hdid, Real &hnext,
                                 DerivFunc derivs)
        {
            // Quality-controlled Runge-Kutta step
            // Uses Cash-Karp method with adaptive step size

            constexpr Real SAFETY = 0.9;
            constexpr Real PGROW = -0.2;
            constexpr Real PSHRNK = -0.25;
            constexpr Real ERRCON = 1.89e-4; // (5/SAFETY)^(1/PGROW)

            RealVector yerr(n);
            RealVector ytemp(n);
            Real h = htry;

            // Iterate until step is accepted
            while (true)
            {
                // Take a Cash-Karp step
                rkck(y, dydx, n, x, h, ytemp, yerr, derivs);

                // Evaluate error
                Real errmax = 0.0;
                for (int i = 0; i < n; ++i)
                {
                    errmax = std::max(errmax, std::abs(yerr[i] / yscal[i]));
                }
                errmax /= eps;

                if (errmax <= 1.0)
                {
                    // Step succeeded
                    hdid = h;
                    if (errmax > ERRCON)
                    {
                        hnext = SAFETY * h * std::pow(errmax, PGROW);
                    }
                    else
                    {
                        hnext = 5.0 * h; // No more than factor of 5 increase
                    }
                    x += h;
                    for (int i = 0; i < n; ++i)
                    {
                        y[i] = ytemp[i];
                    }
                    return true;
                }

                // Step failed, reduce step size
                Real htemp = SAFETY * h * std::pow(errmax, PSHRNK);
                h = (h >= 0.0) ? std::max(htemp, 0.1 * h) : std::min(htemp, 0.1 * h);

                Real xnew = x + h;
                if (xnew == x)
                {
                    error_message_ = "Stepsize underflow in rkqs";
                    return false;
                }
            }
        }

        void ODEIntegrator::rkck(const RealVector &y, const RealVector &dydx, int n,
                                 Real x, Real h,
                                 RealVector &yout, RealVector &yerr,
                                 DerivFunc derivs)
        {
            // Cash-Karp Runge-Kutta step
            // 5th order method with embedded 4th order error estimate

            // Cash-Karp parameters
            constexpr Real A2 = 0.2, A3 = 0.3, A4 = 0.6, A5 = 1.0, A6 = 0.875;
            constexpr Real B21 = 0.2;
            constexpr Real B31 = 3.0 / 40.0, B32 = 9.0 / 40.0;
            constexpr Real B41 = 0.3, B42 = -0.9, B43 = 1.2;
            constexpr Real B51 = -11.0 / 54.0, B52 = 2.5, B53 = -70.0 / 27.0, B54 = 35.0 / 27.0;
            constexpr Real B61 = 1631.0 / 55296.0, B62 = 175.0 / 512.0, B63 = 575.0 / 13824.0;
            constexpr Real B64 = 44275.0 / 110592.0, B65 = 253.0 / 4096.0;
            constexpr Real C1 = 37.0 / 378.0, C3 = 250.0 / 621.0, C4 = 125.0 / 594.0, C6 = 512.0 / 1771.0;
            constexpr Real DC1 = C1 - 2825.0 / 27648.0;
            constexpr Real DC3 = C3 - 18575.0 / 48384.0;
            constexpr Real DC4 = C4 - 13525.0 / 55296.0;
            constexpr Real DC5 = -277.0 / 14336.0;
            constexpr Real DC6 = C6 - 0.25;

            RealVector ak2(n), ak3(n), ak4(n), ak5(n), ak6(n), ytemp(n);

            // First step
            for (int i = 0; i < n; ++i)
            {
                ytemp[i] = y[i] + B21 * h * dydx[i];
            }
            derivs(x + A2 * h, ytemp, ak2);

            // Second step
            for (int i = 0; i < n; ++i)
            {
                ytemp[i] = y[i] + h * (B31 * dydx[i] + B32 * ak2[i]);
            }
            derivs(x + A3 * h, ytemp, ak3);

            // Third step
            for (int i = 0; i < n; ++i)
            {
                ytemp[i] = y[i] + h * (B41 * dydx[i] + B42 * ak2[i] + B43 * ak3[i]);
            }
            derivs(x + A4 * h, ytemp, ak4);

            // Fourth step
            for (int i = 0; i < n; ++i)
            {
                ytemp[i] = y[i] + h * (B51 * dydx[i] + B52 * ak2[i] + B53 * ak3[i] + B54 * ak4[i]);
            }
            derivs(x + A5 * h, ytemp, ak5);

            // Fifth step
            for (int i = 0; i < n; ++i)
            {
                ytemp[i] = y[i] + h * (B61 * dydx[i] + B62 * ak2[i] + B63 * ak3[i] + B64 * ak4[i] + B65 * ak5[i]);
            }
            derivs(x + A6 * h, ytemp, ak6);

            // Accumulate increments with proper weights (5th order)
            for (int i = 0; i < n; ++i)
            {
                yout[i] = y[i] + h * (C1 * dydx[i] + C3 * ak3[i] + C4 * ak4[i] + C6 * ak6[i]);
            }

            // Estimate error as difference between 4th and 5th order methods
            for (int i = 0; i < n; ++i)
            {
                yerr[i] = h * (DC1 * dydx[i] + DC3 * ak3[i] + DC4 * ak4[i] + DC5 * ak5[i] + DC6 * ak6[i]);
            }
        }

    } // namespace math
} // namespace bladenoise
