#include "bladenoise/airfoil/AirfoilGeometryAnalyzer.h"
#include "bladenoise/core/Constants.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace bladenoise {
namespace airfoil {

using namespace constants;

bool AirfoilGeometryAnalyzer::analyze(const io::AirfoilData& airfoil,
                                       AirfoilGeometry& geometry)
{
    if (airfoil.num_points < 10) {
        error_message_ = "Insufficient airfoil points for analysis";
        return false;
    }

    // Find leading edge
    int le_idx = find_leading_edge_index(airfoil);
    if (le_idx < 0) {
        error_message_ = "Could not find leading edge";
        return false;
    }
    geometry.leading_edge_index = le_idx;
    geometry.leading_edge_x = airfoil.x[le_idx];
    geometry.leading_edge_y = airfoil.y[le_idx];

    // Split into upper and lower surfaces
    RealVector x_upper, y_upper, x_lower, y_lower;
    split_surfaces(airfoil, le_idx, x_upper, y_upper, x_lower, y_lower);

    if (x_upper.size() < 3 || x_lower.size() < 3) {
        error_message_ = "Insufficient points on upper or lower surface";
        return false;
    }

    // Trailing edge properties
    geometry.upper_te_index = 0;
    geometry.lower_te_index = static_cast<int>(airfoil.num_points) - 1;

    Real y_upper_te = airfoil.y[geometry.upper_te_index];
    Real y_lower_te = airfoil.y[geometry.lower_te_index];

    geometry.trailing_edge_x = (airfoil.x[geometry.upper_te_index] +
                                airfoil.x[geometry.lower_te_index]) / 2.0;
    geometry.trailing_edge_y = (y_upper_te + y_lower_te) / 2.0;
    geometry.trailing_edge_thickness = std::abs(y_upper_te - y_lower_te);

    // Calculate trailing edge angle (PSI) - improved method
    geometry.trailing_edge_angle = calculate_trailing_edge_angle(airfoil);

    // Calculate thickness distribution
    Real max_t = 0.0;
    Real max_t_x = 0.0;

    for (Real x = 0.05; x <= 0.95; x += 0.01) {
        Real t = calculate_thickness_at(airfoil, x);
        if (t > max_t) {
            max_t = t;
            max_t_x = x;
        }
    }
    geometry.max_thickness = max_t;
    geometry.max_thickness_location = max_t_x;

    // Thickness at specific locations
    geometry.thickness_at_1_percent = calculate_thickness_at(airfoil, 0.01);
    geometry.thickness_at_10_percent = calculate_thickness_at(airfoil, 0.10);

    // Calculate camber (mean line)
    Real max_camber = 0.0;
    Real max_camber_x = 0.0;

    for (Real x = 0.05; x <= 0.95; x += 0.01) {
        Real y_u = interpolate_y(x_upper, y_upper, x);
        Real y_l = interpolate_y(x_lower, y_lower, x);
        Real camber = (y_u + y_l) / 2.0;
        if (std::abs(camber) > std::abs(max_camber)) {
            max_camber = camber;
            max_camber_x = x;
        }
    }
    geometry.max_camber = max_camber;
    geometry.max_camber_location = max_camber_x;

    // Estimate leading edge radius
    if (x_upper.size() >= 3 && x_lower.size() >= 3) {
        Real x1 = x_upper[x_upper.size() - 1];
        Real y1 = y_upper[y_upper.size() - 1];
        Real x2 = x_upper[x_upper.size() - 2];
        Real y2 = y_upper[y_upper.size() - 2];
        Real x3 = x_lower[1];
        Real y3 = y_lower[1];

        Real a = std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
        Real b = std::sqrt((x3-x2)*(x3-x2) + (y3-y2)*(y3-y2));
        Real c = std::sqrt((x1-x3)*(x1-x3) + (y1-y3)*(y1-y3));
        Real s = (a + b + c) / 2.0;
        Real area = std::sqrt(std::max(s * (s-a) * (s-b) * (s-c), 0.0));

        if (area > 1e-10) {
            geometry.leading_edge_radius = (a * b * c) / (4.0 * area);
        }
    }

    return true;
}

Real AirfoilGeometryAnalyzer::calculate_trailing_edge_angle(const io::AirfoilData& airfoil)
{
    // =================================================================
    // Improved trailing edge angle (PSI) calculation
    //
    // The TE angle is the angle between the upper and lower surface
    // tangent lines at the trailing edge. This is important for the
    // bluntness noise calculation.
    //
    // We use a least-squares fit of the last few points on each surface
    // to get a robust slope estimate, rather than just using 2 points.
    // =================================================================

    if (airfoil.num_points < 10) {
        return 14.0;  // Default
    }

    int n = static_cast<int>(airfoil.num_points);
    int le_idx = find_leading_edge_index(airfoil);

    // Number of points to use for slope estimation near TE
    // Use points in the last 5% of chord on each surface
    int n_fit = std::min(5, le_idx / 2);
    n_fit = std::max(n_fit, 2);

    // Upper surface slope at TE (points near index 0)
    // Fit a line through the first n_fit points
    Real sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    for (int i = 0; i < n_fit; ++i) {
        Real x = airfoil.x[i];
        Real y = airfoil.y[i];
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }
    Real slope_upper = (n_fit * sum_xy - sum_x * sum_y) /
                       (n_fit * sum_xx - sum_x * sum_x + 1e-20);

    // Lower surface slope at TE (points near index n-1)
    sum_x = 0; sum_y = 0; sum_xx = 0; sum_xy = 0;
    for (int i = 0; i < n_fit; ++i) {
        int idx = n - 1 - i;
        Real x = airfoil.x[idx];
        Real y = airfoil.y[idx];
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }
    Real slope_lower = (n_fit * sum_xy - sum_x * sum_y) /
                       (n_fit * sum_xx - sum_x * sum_x + 1e-20);

    // The TE angle is the angle between the two tangent lines
    Real angle_upper = std::atan(slope_upper);
    Real angle_lower = std::atan(slope_lower);

    // PSI = angle between upper and lower surface at TE
    // For typical airfoils: upper surface slopes down (negative dy/dx at TE)
    // and lower surface slopes up (positive dy/dx at TE)
    Real psi_rad = std::abs(angle_upper - angle_lower);
    Real psi_deg = psi_rad * RAD_TO_DEG;

    // Clamp to reasonable values
    psi_deg = std::max(0.0, std::min(90.0, psi_deg));

    return psi_deg;
}

Real AirfoilGeometryAnalyzer::calculate_trailing_edge_thickness(const io::AirfoilData& airfoil)
{
    if (airfoil.num_points < 2) {
        return 0.0;
    }

    Real y_upper = airfoil.y[0];
    Real y_lower = airfoil.y[airfoil.num_points - 1];

    return std::abs(y_upper - y_lower);
}

Real AirfoilGeometryAnalyzer::calculate_thickness_at(const io::AirfoilData& airfoil,
                                                      Real x_over_c)
{
    int le_idx = find_leading_edge_index(airfoil);
    if (le_idx < 0) return 0.0;

    RealVector x_upper, y_upper, x_lower, y_lower;
    split_surfaces(airfoil, le_idx, x_upper, y_upper, x_lower, y_lower);

    if (x_upper.empty() || x_lower.empty()) return 0.0;

    Real y_u = interpolate_y(x_upper, y_upper, x_over_c);
    Real y_l = interpolate_y(x_lower, y_lower, x_over_c);

    return std::abs(y_u - y_l);
}

int AirfoilGeometryAnalyzer::find_leading_edge_index(const io::AirfoilData& airfoil)
{
    if (airfoil.num_points == 0) return -1;

    int le_idx = 0;
    Real min_x = airfoil.x[0];

    for (size_t i = 1; i < airfoil.num_points; ++i) {
        if (airfoil.x[i] < min_x) {
            min_x = airfoil.x[i];
            le_idx = static_cast<int>(i);
        }
    }

    return le_idx;
}

void AirfoilGeometryAnalyzer::split_surfaces(const io::AirfoilData& airfoil, int le_index,
                                              RealVector& x_upper, RealVector& y_upper,
                                              RealVector& x_lower, RealVector& y_lower)
{
    x_upper.clear();
    y_upper.clear();
    x_lower.clear();
    y_lower.clear();

    // Upper surface (reverse order: TE to LE -> LE to TE)
    for (int i = le_index; i >= 0; --i) {
        x_upper.push_back(airfoil.x[i]);
        y_upper.push_back(airfoil.y[i]);
    }

    // Lower surface (LE to TE)
    for (size_t i = le_index; i < airfoil.num_points; ++i) {
        x_lower.push_back(airfoil.x[i]);
        y_lower.push_back(airfoil.y[i]);
    }
}

Real AirfoilGeometryAnalyzer::interpolate_y(const RealVector& x, const RealVector& y,
                                             Real x_target)
{
    if (x.size() < 2) return 0.0;

    // Find bracketing indices
    size_t i = 0;
    while (i < x.size() - 1 && x[i+1] < x_target) {
        ++i;
    }

    if (i >= x.size() - 1) {
        return y.back();
    }

    // Linear interpolation
    Real dx = x[i+1] - x[i];
    if (std::abs(dx) < 1e-12) return y[i];

    Real t = (x_target - x[i]) / dx;
    t = std::max(0.0, std::min(1.0, t));
    return y[i] + t * (y[i+1] - y[i]);
}

Real AirfoilGeometryAnalyzer::calculate_slope(const RealVector& x, const RealVector& y,
                                               int index, bool forward)
{
    int n = static_cast<int>(x.size());
    if (n < 2 || index < 0 || index >= n) return 0.0;

    if (forward) {
        if (index >= n - 1) return 0.0;
        Real dx = x[index+1] - x[index];
        if (std::abs(dx) < 1e-12) return 0.0;
        return (y[index+1] - y[index]) / dx;
    } else {
        if (index <= 0) return 0.0;
        Real dx = x[index] - x[index-1];
        if (std::abs(dx) < 1e-12) return 0.0;
        return (y[index] - y[index-1]) / dx;
    }
}

std::unique_ptr<AirfoilGeometryAnalyzer> create_geometry_analyzer()
{
    return std::make_unique<AirfoilGeometryAnalyzer>();
}

}  // namespace airfoil
}  // namespace bladenoise
