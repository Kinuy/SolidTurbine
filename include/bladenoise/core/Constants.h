#pragma once

#include "bladenoise/core/Types.h"
#include <cmath>
#include <array>

namespace bladenoise {
namespace constants {

// Mathematical constants
constexpr Real PI = 3.14159265358979323846;
constexpr Real TWO_PI = 2.0 * PI;
constexpr Real HALF_PI = PI / 2.0;
constexpr Real DEG_TO_RAD = PI / 180.0;
constexpr Real RAD_TO_DEG = 180.0 / PI;

// Physical constants (ISA sea level defaults)
constexpr Real DEFAULT_SPEED_OF_SOUND = 340.46;        // m/s
constexpr Real DEFAULT_KINEMATIC_VISCOSITY = 1.4607e-5; // m^2/s
constexpr Real DEFAULT_AIR_DENSITY = 1.225;             // kg/m^3

// Reference pressure for dB calculations
constexpr Real P_REF = 2.0e-5;  // Pa (20 micro-Pa)
constexpr Real P_REF_SQ = P_REF * P_REF;

// One-third octave band center frequencies (Hz)
// Standard 34 bands from 10 Hz to 20 kHz (matching NAFNoise)
constexpr std::array<Real, 34> THIRD_OCTAVE_BANDS = {
    10.0, 12.5, 16.0, 20.0, 25.0, 31.5, 40.0,
    50.0, 63.0, 80.0, 100.0, 125.0, 160.0, 200.0,
    250.0, 315.0, 400.0, 500.0, 630.0, 800.0,
    1000.0, 1250.0, 1600.0, 2000.0, 2500.0, 3150.0,
    4000.0, 5000.0, 6300.0, 8000.0, 10000.0, 12500.0,
    16000.0, 20000.0
};

}  // namespace constants
}  // namespace bladenoise
