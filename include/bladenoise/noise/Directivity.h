#pragma once

#include "bladenoise/core/Types.h"

namespace bladenoise {
namespace noise {

class Directivity {
public:
    // High-frequency directivity (BPM Eq. 1)
    static Real high_frequency(Real mach, Real theta, Real phi);

    // Low-frequency directivity (BPM Eq. 2)
    static Real low_frequency(Real mach, Real theta, Real phi);
};

}  // namespace noise
}  // namespace bladenoise
