#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/io/IOTypes.h"
#include <memory>
#include <string>

namespace bladenoise {
namespace airfoil {

// Computed airfoil geometry properties
struct AirfoilGeometry {
    // Leading edge
    int leading_edge_index = 0;
    Real leading_edge_x = 0.0;
    Real leading_edge_y = 0.0;
    Real leading_edge_radius = 0.0;

    // Trailing edge
    int upper_te_index = 0;
    int lower_te_index = 0;
    Real trailing_edge_x = 1.0;
    Real trailing_edge_y = 0.0;
    Real trailing_edge_thickness = 0.0;  // Normalized by chord
    Real trailing_edge_angle = 0.0;      // PSI in degrees

    // Thickness
    Real max_thickness = 0.0;
    Real max_thickness_location = 0.0;
    Real thickness_at_1_percent = 0.0;
    Real thickness_at_10_percent = 0.0;

    // Camber
    Real max_camber = 0.0;
    Real max_camber_location = 0.0;
};

class AirfoilGeometryAnalyzer {
public:
    bool analyze(const io::AirfoilData& airfoil, AirfoilGeometry& geometry);
    std::string get_error() const { return error_message_; }

    // Individual analysis functions (public for reuse)
    static Real calculate_trailing_edge_angle(const io::AirfoilData& airfoil);
    static Real calculate_trailing_edge_thickness(const io::AirfoilData& airfoil);
    static Real calculate_thickness_at(const io::AirfoilData& airfoil, Real x_over_c);

    static int find_leading_edge_index(const io::AirfoilData& airfoil);
    static void split_surfaces(const io::AirfoilData& airfoil, int le_index,
                               RealVector& x_upper, RealVector& y_upper,
                               RealVector& x_lower, RealVector& y_lower);

private:
    static Real interpolate_y(const RealVector& x, const RealVector& y, Real x_target);
    static Real calculate_slope(const RealVector& x, const RealVector& y,
                                int index, bool forward);

    std::string error_message_;
};

// Factory
std::unique_ptr<AirfoilGeometryAnalyzer> create_geometry_analyzer();

}  // namespace airfoil
}  // namespace bladenoise
