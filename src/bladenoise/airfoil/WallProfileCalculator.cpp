#include "bladenoise/airfoil/WallProfileCalculator.h"
#include "bladenoise/core/Constants.h"
#include <cmath>
#include <algorithm>

namespace bladenoise {
namespace airfoil {

using namespace constants;

Real WallProfileCalculator::calculate_skin_friction(
    Real HK, Real RT, Real MSQ,
    Real& CF_HK, Real& CF_RT, Real& CF_MSQ)
{
    constexpr Real GAM = 1.4;
    Real GM1 = GAM - 1.0;
    Real FC = std::sqrt(1.0 + 0.5 * GM1 * MSQ);
    Real GRT = std::max(std::log(RT / FC), 3.0);
    Real GEX = -1.74 - 0.31 * HK;
    Real ARG = std::max(-1.33 * HK, -20.0);
    Real THK = std::tanh(4.0 - HK / 0.875);

    Real CFO = 0.3 * std::exp(ARG) * std::pow(GRT / 2.3026, GEX);
    Real CF = (CFO + 1.1e-4 * (THK - 1.0)) / FC;
    
    CF_HK = (-1.33 * CFO - 0.31 * std::log(GRT / 2.3026) * CFO - 
             1.1e-4 * (1.0 - THK * THK) / 0.875) / FC;
    CF_RT = GEX * CFO / (FC * GRT * RT);
    CF_MSQ = GEX * CFO / (FC * GRT) * (-0.25 * GM1 / (FC * FC)) - 
             0.25 * GM1 * CF / (FC * FC);

    return CF;
}

WallProfileCalculator::ProfileResult WallProfileCalculator::calculate(
    Real DSTAR, Real THETA, Real /*UO_in*/, Real RT, Real MS)
{
    constexpr Real AK = 0.09;
    constexpr Real VKAP = 0.40;
    constexpr Real VB = 5.0;
    
    ProfileResult result;
    
    Real HK = DSTAR / THETA;
    Real UO = 1.0;
    result.wake_exponent = 1.0;
    
    Real CF_HK, CF_RT, CF_MS;
    Real CF = calculate_skin_friction(HK, RT, MS, CF_HK, CF_RT, CF_MS);
    
    Real SGN = (CF >= 0.0) ? 1.0 : -1.0;
    Real UT = SGN * std::sqrt(0.5 * std::abs(CF));
    Real UI = std::min(UT / AK * HALF_PI, 0.90);
    Real DO = HK * THETA / (1.0 - 0.5 * (UO + UI));
    Real EBK = std::exp(-VB * VKAP);

    // Simplified iteration for convergence
    for (int iter = 0; iter < 12; ++iter) {
        SGN = (UT >= 0.0) ? 1.0 : -1.0;
        Real DP = SGN * UT * RT * DO / THETA;
        
        // Spalding formula iteration
        Real UPE = std::log(std::max(DP, 1e-10)) / VKAP + VB;
        for (int itup = 0; itup < 5; ++itup) {
            Real UK = UPE * VKAP;
            Real ARG = UK - VB * VKAP;
            Real EXU = std::exp(ARG);
            Real REZ = UPE + EXU - EBK * (1.0 + UK + UK*UK/2.0 + UK*UK*UK/6.0) - DP;
            Real DP_U = 1.0 + (EXU - EBK * (1.0 + UK + UK*UK/2.0)) * VKAP;
            if (std::abs(REZ / std::max(DP, 1e-10)) < 1.0e-5) break;
            UPE -= REZ / DP_U;
        }
        
        Real DUO = UO - UT * UPE;
        DO = HK * THETA / (1.0 - 0.5 * (UO + (UO - DUO)));
    }

    result.boundary_layer_delta = DO;
    result.inner_velocity = UI;
    result.skin_friction = CF;
    
    return result;
}

}  // namespace airfoil
}  // namespace bladenoise
