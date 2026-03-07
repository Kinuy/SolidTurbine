#include "bladenoise/noise/Directivity.h"
#include "bladenoise/core/Constants.h"
#include <cmath>

namespace bladenoise {
namespace noise {

using namespace constants;

Real Directivity::high_frequency(Real mach, Real theta, Real phi) {
    // Convert angles to radians
    Real theta_rad = theta * DEG_TO_RAD;
    Real phi_rad = phi * DEG_TO_RAD;
    
    // Convective amplification factor
    Real mc = 0.8 * mach;  // Convection Mach number
    
    Real cos_theta = std::cos(theta_rad);
    Real sin_theta = std::sin(theta_rad);
    Real sin_phi = std::sin(phi_rad);
    
    // High-frequency directivity (BPM Eq. 1)
    Real numerator = 2.0 * sin_theta * sin_theta * sin_phi * sin_phi;
    Real denominator = std::pow(1.0 + mach * cos_theta, 4) * 
                       std::pow(1.0 + (mach - mc) * cos_theta, 2);
    
    // Avoid division by zero
    if (denominator < 1e-10) {
        return 0.0;
    }
    
    return numerator / denominator;
}

Real Directivity::low_frequency(Real mach, Real theta, Real phi) {
    // Convert angles to radians
    Real theta_rad = theta * DEG_TO_RAD;
    Real phi_rad = phi * DEG_TO_RAD;
    
    Real cos_theta = std::cos(theta_rad);
    Real sin_theta = std::sin(theta_rad);
    Real sin_phi = std::sin(phi_rad);
    
    // Low-frequency directivity (BPM Eq. 2)
    Real numerator = sin_theta * sin_theta * sin_phi * sin_phi;
    Real denominator = std::pow(1.0 + mach * cos_theta, 4);
    
    // Avoid division by zero
    if (denominator < 1e-10) {
        return 0.0;
    }
    
    return numerator / denominator;
}

}  // namespace noise
}  // namespace bladenoise
