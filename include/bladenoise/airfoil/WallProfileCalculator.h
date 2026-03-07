#pragma once

#include "bladenoise/core/Types.h"

namespace bladenoise {
namespace airfoil {

class WallProfileCalculator {
public:
    struct ProfileResult {
        Real boundary_layer_delta = 0.0;
        Real inner_velocity = 0.0;
        Real skin_friction = 0.0;
        Real wake_exponent = 1.0;
    };

    static Real calculate_skin_friction(
        Real HK, Real RT, Real MSQ,
        Real& CF_HK, Real& CF_RT, Real& CF_MSQ);

    ProfileResult calculate(
        Real DSTAR, Real THETA, Real UO_in, Real RT, Real MS);
};

}  // namespace airfoil
}  // namespace bladenoise
