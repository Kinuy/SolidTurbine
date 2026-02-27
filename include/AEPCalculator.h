#pragma once
/**
 * @file AEPCalculator.h
 * @brief Computes Annual Energy Production using IEC 61400-12-1 (Weibull method).
 *
 * Single Responsibility: AEP calculation only.
 */
#include <vector>

class AEPCalculator
{
public:
    struct AEPResult
    {
        double aep_kwh;   ///< Annual energy production [kWh/a]
        double aep_money; ///< Annual revenue [currency/a]
    };

    AEPCalculator(std::vector<double> vinf_vec,
                  std::vector<double> pel_vec,
                  double dv,
                  double k,
                  double money_per_kwh);

    /// Compute AEP for a given mean wind speed (Weibull distribution).
    AEPResult Compute(double v_mean) const;

    /// Compute AEP range for a given mean wind speed range (Weibull distribution).
    std::vector<AEPResult> ComputeRange(
        std::vector<double> const &vmean_vec) const;

private:
    std::vector<double> vinf_;
    std::vector<double> pel_;
    double dv_;
    double k_;
    double money_;

    static double WeibullBin(double v, double dv, double A, double k);
};