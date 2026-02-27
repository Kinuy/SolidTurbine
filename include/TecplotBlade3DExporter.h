#pragma once
/**
 * @file TecplotBlade3DExporter.h
 * @brief Exports 3D blade section geometry in Tecplot format.
 *
 * SOLID compliance:
 *  S – responsible only for mapping blade geometry → DataFormat.
 *      Formatting and I/O delegated to IFormatter / FileOutputTarget.
 *  O – new formats: implement IBlade3DExporter with a different IFormatter.
 *  L – fully satisfies IBlade3DExporter.
 *  I – consumers see only IBlade3DExporter.
 *  D – depends on IFormatter abstraction, not TecplotFormatter directly.
 *
 * Tecplot file layout
 * ───────────────────
 * TITLE = "blade_3d_geometry"
 * VARIABLES = "X[m]" "Y[m]" "Z[m]" "chord[m]" "radius[m]" "rel_thickness[%]"
 *
 * One ZONE per blade section (named by airfoil name + radius):
 *   ZONE I=<n_points>, T="<name>_r<radius>"
 *   x0 y0 z0 chord radius thickness
 *   x1 y1 z1 ...
 *   ...
 *
 * Tecplot can animate over zones to visualise the blade sweep,
 * or load all zones simultaneously for a 3D surface view.
 */
#include "IBlade3DExporter.h"
#include "IFormatter.h"
#include "DataFormat.h"
#include <memory>

class TecplotBlade3DExporter final : public IBlade3DExporter
{
public:
    explicit TecplotBlade3DExporter(std::shared_ptr<IFormatter> formatter);

    bool Export(BladeInterpolator const &interpolator,
                std::string const &output_path) const override;

private:
    std::shared_ptr<IFormatter> formatter_;

    /// Build a DataFormat from all blade sections.
    static DataFormat BuildFormat(BladeInterpolator const &interpolator);
};
