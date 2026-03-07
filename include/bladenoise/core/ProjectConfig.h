#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/Constants.h"
#include <string>

namespace bladenoise {

struct ProjectConfig {
    // Atmospheric properties
    Real speed_of_sound = constants::DEFAULT_SPEED_OF_SOUND;
    Real kinematic_viscosity = constants::DEFAULT_KINEMATIC_VISCOSITY;
    Real air_density = constants::DEFAULT_AIR_DENSITY;

    // Airfoil / blade element properties
    Real chord = 1.0;                  // Chord length (m)
    Real span = 1.0;                   // Span / element length (m)
    Real freestream_velocity = 71.3;   // Freestream velocity (m/s)
    Real angle_of_attack = 0.0;        // Angle of attack (degrees)

    // Trailing edge geometry
    Real trailing_edge_thickness = 0.0; // TE thickness (m)
    Real trailing_edge_angle = 14.0;    // TE solid angle PSI (degrees)

    // Boundary layer configuration
    TripConfig trip_config = TripConfig::NO_TRIP;
    BoundaryLayerMethod bl_method = BoundaryLayerMethod::BPM;

    // Transition locations (x/c, for XFOIL method)
    Real xtr_upper = 1.0;  // Upper surface forced transition
    Real xtr_lower = 1.0;  // Lower surface forced transition

    // Noise method selection
    TBLNoiseMethod tbl_method = TBLNoiseMethod::BPM;
    TINoiseMethod ti_method = TINoiseMethod::NONE;

    // Noise computation flags
    bool compute_bluntness = true;
    bool compute_laminar = true;

    // Turbulent inflow parameters
    Real turbulence_intensity = 0.0;      // Percent
    Real turbulence_length_scale = 0.01;  // m

    // Thickness at specific chord locations (for TI noise)
    Real thickness_1_percent = 0.0;   // t/c at 1% chord
    Real thickness_10_percent = 0.0;  // t/c at 10% chord

    // Airfoil file
    std::string airfoil_file = "airfoil.dat";
    bool is_naca = false;
    bool auto_calculate_geometry = true;

    // Observer position
    Real observer_distance = 1.22;     // m
    Real observer_theta = 90.0;        // Degrees from chordline
    Real observer_phi = 90.0;          // Degrees from spanline

    // Streamline output
    int num_streamlines = 5;
    Real streamline_spacing = 0.01;
    bool write_streamlines = false;
    std::string streamline_output_file = "streamlines.dat";
    std::string streamline_output_format = "text";

    // Derived quantities
    Real reynolds_number() const {
        return freestream_velocity * chord / kinematic_viscosity;
    }

    Real mach_number() const {
        return freestream_velocity / speed_of_sound;
    }

    // Validation
    bool validate() const;
};

}  // namespace bladenoise
