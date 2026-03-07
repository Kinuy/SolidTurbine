#pragma once

#include <vector>
#include <complex>
#include <string>

namespace bladenoise
{

    // Floating point precision (change to float for single precision)
    using Real = double;
    using Complex = std::complex<double>;
    using RealVector = std::vector<Real>;

    // Enumerations for configuration

    enum class TripConfig
    {
        NO_TRIP = 0,
        HEAVY_TRIP = 1,
        LIGHT_TRIP = 2
    };

    enum class BoundaryLayerMethod
    {
        BPM = 1,
        XFOIL = 2
    };

    enum class TBLNoiseMethod
    {
        NONE = 0,
        BPM = 1,
        TNO = 2
    };

    enum class TINoiseMethod
    {
        NONE = 0,
        AMIET = 1,
        GUIDATI = 2,
        SIMPLIFIED = 3
    };

    // Boundary layer state at a station (typically trailing edge)
    struct BoundaryLayerState
    {
        Real boundary_layer_thickness = 0.0; // delta
        Real displacement_thickness = 0.0;   // delta*
        Real momentum_thickness = 0.0;       // theta
        Real shape_factor = 0.0;             // H = delta*/theta
        Real skin_friction = 0.0;            // Cf
        Real edge_velocity = 0.0;            // Ue
        Real reynolds_theta = 0.0;           // Re_theta
        bool is_turbulent = false;
    };

    // Noise result for a single noise source
    struct NoiseResult
    {
        RealVector spl;         // SPL per frequency band
        Real overall_spl = 0.0; // Overall SPL (OASPL)

        NoiseResult() = default;
        explicit NoiseResult(size_t n) : spl(n, -100.0), overall_spl(-100.0) {}
    };

    // Combined noise results from all sources
    struct CombinedNoiseResults
    {
        NoiseResult tbl_pressure_side;
        NoiseResult tbl_suction_side;
        NoiseResult separation;
        NoiseResult laminar_vortex;
        NoiseResult bluntness;
        NoiseResult turbulent_inflow;
        NoiseResult total;
    };

} // namespace bladenoise
