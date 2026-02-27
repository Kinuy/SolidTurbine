/**
 * @file TecplotBlade3DExporter.cpp
 *
 * Coordinate frame (mirrors DXFBlade3D exactly):
 *
 *   X  — chordwise direction in the rotor plane  [m]
 *         populated by applyScalingWithChordAndMaxThickness (coord.x *= chord)
 *   Y  — thickness direction (perpendicular to chord in section plane)  [m]
 *         populated by applyScalingWithChordAndMaxThickness (coord.y *= maxThickness)
 *   Z  — spanwise / radial direction  [m]
 *         populated by applyScalingWithChordAndMaxThickness (coord.z *= targetRadius)
 *
 * Each blade section becomes one Tecplot ZONE I=<n+1> where the last point
 * repeats the first to explicitly close the airfoil contour — matching the
 * "closed=true" flag that DXFPolyLine uses.
 *
 * Tecplot layout:
 *   TITLE = "blade_3d_geometry"
 *   VARIABLES = "X_chord[m]" "Y_thick[m]" "Z_radius[m]" "chord[m]" "radius[m]" "rel_thickness[%]"
 *
 *   ZONE I=<npts+1>, T="<airfoilName>_r<radius>m"
 *   x0  y0  z0  chord  radius  thickness
 *   x1  y1  z1  ...
 *   ...
 *   x0  y0  z0  ...        <- repeated to close contour
 */
#include "TecplotBlade3DExporter.h"
#include "BladeInterpolator.h"
#include "DataWriter.h"
#include "FileOutputTarget.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
TecplotBlade3DExporter::TecplotBlade3DExporter(
    std::shared_ptr<IFormatter> formatter)
    : formatter_(std::move(formatter))
{
    if (!formatter_)
        throw std::invalid_argument(
            "TecplotBlade3DExporter: formatter must be non-null");
}

// ─────────────────────────────────────────────────────────────────────────────
// Export
// ─────────────────────────────────────────────────────────────────────────────
bool TecplotBlade3DExporter::Export(BladeInterpolator const &interpolator,
                                    std::string const &output_path) const
{
    namespace fs = std::filesystem;

    fs::path file_path(output_path);
    fs::path dir = file_path.parent_path();
    if (!dir.empty() && !fs::exists(dir))
    {
        std::error_code ec;
        fs::create_directories(dir, ec);
        if (ec)
        {
            std::cerr << "TecplotBlade3DExporter: cannot create directory '"
                      << dir << "': " << ec.message() << '\n';
            return false;
        }
    }

    if (fs::exists(file_path))
        fs::remove(file_path);

    DataFormat fmt = BuildFormat(interpolator);
    auto data = std::make_shared<DataFormat>(fmt);
    auto out = std::make_shared<FileOutputTarget>(file_path);
    DataWriter writer(data, formatter_, out);
    return writer.write();
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildFormat
//
// Mirrors DXFBlade3D::fillDxfPolyLineList() exactly:
//   - iterates bladeSections
//   - calls getScaledAndRotatedCoordinates() per section  (same as DXF)
//   - one zone per section, I = n_points + 1 (last point closes the contour)
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotBlade3DExporter::BuildFormat(
    BladeInterpolator const &interpolator)
{
    DataFormat fmt("blade_3d_geometry");
    fmt.setVariables({
        "X_chord[m]",      // chordwise  — coord.x * chord
        "Y_thick[m]",      // thickness  — coord.y * maxThickness
        "Z_radius[m]",     // spanwise   — coord.z * targetRadius
        "chord[m]",        // section chord (constant within zone)
        "radius[m]",       // section radius (constant within zone)
        "rel_thickness[%]" // relative thickness (constant within zone)
    });

    auto const &sections = interpolator.getBladeSections();

    for (std::size_t s = 0; s < sections.size(); ++s)
    {
        auto const &sec = sections.at(s);

        double chord = sec->chord;
        double radius = sec->bladeRadius;
        double thick = sec->relativeThickness;

        // Same coordinate source as DXFBlade3D
        auto const &coords =
            sec->airfoilGeometry->getScaledAndRotatedCoordinates();

        if (coords.empty())
            continue;

        // Zone title mirrors DXF convention
        std::ostringstream zone_title;
        zone_title << sec->airfoilName
                   << "_r" << std::fixed << std::setprecision(2) << radius << "m";

        // I = n_points + 1  — extra point closes the contour (like DXF closed=true)
        DataZone zone(zone_title.str(),
                      static_cast<int>(coords.size()) + 1);

        // Fill contour points
        for (auto const &pt : coords)
        {
            zone.data.push_back({pt.x, // X_chord
                                 pt.y, // Y_thick
                                 pt.z, // Z_radius
                                 chord,
                                 radius,
                                 thick});
        }

        // Closing point — repeat first coordinate (mirrors DXF closed polyline)
        auto const &first = coords.front();
        zone.data.push_back({first.x, first.y, first.z, chord, radius, thick});

        fmt.addZone(zone);
    }

    return fmt;
}
