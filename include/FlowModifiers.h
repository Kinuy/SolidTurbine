#ifndef FLOWMODIFIERS_H_
#define FLOWMODIFIERS_H_

#include <utility>

/// Variables that modify the global flow (shear, veer, etc.).
/// The bool in each pair indicates whether that modifier is active;
/// set by the input case parser when reading the .wvpc file.
struct FlowModifiers
{
    FlowModifiers() = default;

    std::pair<bool, double> veer{false, 0.0};
    std::pair<bool, int> shear{false, 0};

    double surface_roughness{0.0};
    bool use_ref_hv{false};
    double ref_height{0.0};
    double ref_vel{0.0};
    double obukhov_length{500.0};
    double shear_exponent{0.0};
};

#endif // FLOWMODIFIERS_H_