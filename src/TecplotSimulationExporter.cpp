/**
 * @file TecplotSimulationExporter.cpp
 * @brief Implementation of TecplotSimulationExporter.
 *
 * Maps simulation domain objects (PowerCurvePoint, BEMPostprocessResult,
 * TurbineGeometry) to DataFormat/DataZone structures, then delegates
 * formatting and I/O to the injected IFormatter and FileOutputTarget.
 */
#include "TecplotSimulationExporter.h"
#include "TurbineGeometry.h"
#include "DataWriter.h"
#include "FileOutputTarget.h"

#include <numbers>
#include <stdexcept>
#include <filesystem>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
TecplotSimulationExporter::TecplotSimulationExporter(
    std::shared_ptr<IFormatter> formatter)
    : formatter_(std::move(formatter))
{
    if (!formatter_)
        throw std::invalid_argument(
            "TecplotSimulationExporter: formatter must be non-null");
}

// ─────────────────────────────────────────────────────────────────────────────
// Public interface
// ─────────────────────────────────────────────────────────────────────────────
bool TecplotSimulationExporter::ExportPowerCurve(
    std::vector<PowerCurvePoint> const &power_curve,
    std::string const &output_path) const
{
    return Write(BuildPowerCurveFormat(power_curve), output_path);
}

bool TecplotSimulationExporter::ExportBladeData(
    BEMPostprocessResult const &pp,
    TurbineGeometry const *turbine,
    double vinf,
    std::string const &output_path) const
{
    return Write(BuildBladeDataFormat(pp, turbine, vinf), output_path);
}

bool TecplotSimulationExporter::ExportRotorDiscData(
    std::vector<BEMPostprocessResult> const &pp_vec,
    TurbineGeometry const *turbine,
    std::vector<double> const &vinf_vec,
    std::string const &output_path) const
{
    return Write(BuildRotorDiscFormat(pp_vec, turbine, vinf_vec), output_path);
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildPowerCurveFormat
//
// Variables: v_inf, v_tip, pitch, lambda, n, p_wind, p_aero, p_el,
//            cp_aero, ct, torque, eta
// One zone, one row per wind speed.
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotSimulationExporter::BuildPowerCurveFormat(
    std::vector<PowerCurvePoint> const &power_curve)
{
    DataFormat fmt("turbine_performance");
    fmt.setVariables({"v_inf_[m/s]",
                      "v_tip_[m/s]",
                      "pitch_[deg]",
                      "lambda_[-]",
                      "n_[rpm]",
                      "p_wind_[W]",
                      "p_aero_[W]",
                      "p_el_[W]",
                      "cp_aero_[-]",
                      "ct_[-]",
                      "torque_[Nm]",
                      "eta_[-]"});

    DataZone zone("turbine_performance",
                  static_cast<int>(power_curve.size()));

    for (auto const &pt : power_curve)
    {
        zone.data.push_back({pt.vinf,
                             pt.vtip,
                             pt.pitch,
                             pt.lambda,
                             pt.n,
                             pt.p_wind,
                             pt.p_aero,
                             pt.p_el,
                             pt.cp_aero,
                             pt.ct,
                             pt.torque,
                             pt.eta});
    }

    fmt.addZone(zone);
    return fmt;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildBladeDataFormat
//
// Variables: radius, chord, twist, alpha_eff, cl, cd, cm,
//            cp_loc, ct_loc, dT, dQ, dFy, dMz
// One zone, one row per blade section.
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotSimulationExporter::BuildBladeDataFormat(
    BEMPostprocessResult const &pp,
    TurbineGeometry const *turbine,
    double vinf)
{
    DataFormat fmt("blade_data");
    fmt.setVariables({"v_inf_[m/s]",
                      "radius_[m]",
                      "chord_[m]",
                      "twist_[deg]",
                      "alpha_eff_[deg]",
                      "cl_[-]",
                      "cd_[-]",
                      "cm_[-]",
                      "cp_loc_[-]",
                      "ct_loc_[-]",
                      "dT_[N]",
                      "dQ_[Nm]",
                      "dFy_[N]",
                      "dMz_[Nm]"});

    std::size_t n = pp.alpha_eff.size();
    DataZone zone("blade_data", static_cast<int>(n));

    constexpr double rad2deg = 180.0 / std::numbers::pi;

    for (std::size_t i = 0; i < n; ++i)
    {
        double twist_deg = turbine ? turbine->twist(i) * rad2deg : 0.0;
        double chord = turbine ? turbine->chord(i) : 0.0;
        double radius = turbine ? turbine->radius(i) : 0.0;

        zone.data.push_back({vinf,
                             radius,
                             chord,
                             twist_deg,
                             pp.alpha_eff[i] * rad2deg,
                             pp.cl[i],
                             pp.cd[i],
                             pp.cm[i],
                             pp.cp_loc[i],
                             pp.ct_loc[i],
                             pp.element_thrust[i],
                             pp.element_torque[i],
                             pp.element_fy[i],
                             pp.element_mz[i]});
    }

    fmt.addZone(zone);
    return fmt;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildRotorDiscFormat
//
// One zone per wind speed, rows = blade sections.
// Variables identical to blade data — Tecplot can animate over zones.
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotSimulationExporter::BuildRotorDiscFormat(
    std::vector<BEMPostprocessResult> const &pp_vec,
    TurbineGeometry const *turbine,
    std::vector<double> const &vinf_vec)
{
    DataFormat fmt("rotor_disc_data");
    fmt.setVariables({"v_inf_[m/s]",
                      "radius_[m]",
                      "chord_[m]",
                      "twist_[deg]",
                      "alpha_eff_[deg]",
                      "cl_[-]",
                      "cd_[-]",
                      "cm_[-]",
                      "cp_loc_[-]",
                      "ct_loc_[-]",
                      "dT_[N]",
                      "dQ_[Nm]",
                      "dFy_[N]",
                      "dMz_[Nm]",
                      "integral_fx_[N]",
                      "integral_fy_[N]",
                      "integral_mx_[Nm]",
                      "integral_my_[Nm]",
                      "integral_mz_[Nm]"});

    constexpr double rad2deg = 180.0 / std::numbers::pi;

    for (std::size_t v = 0; v < pp_vec.size(); ++v)
    {
        BEMPostprocessResult const &pp = pp_vec[v];
        double vinf = (v < vinf_vec.size()) ? vinf_vec[v] : 0.0;
        std::size_t n = pp.alpha_eff.size();

        DataZone zone("v_inf=" + std::to_string(vinf) + "m_s",
                      static_cast<int>(n));

        for (std::size_t i = 0; i < n; ++i)
        {
            double twist_deg = turbine ? turbine->twist(i) * rad2deg : 0.0;
            double chord = turbine ? turbine->chord(i) : 0.0;
            double radius = turbine ? turbine->radius(i) : 0.0;

            zone.data.push_back({vinf,
                                 radius,
                                 chord,
                                 twist_deg,
                                 pp.alpha_eff[i] * rad2deg,
                                 pp.cl[i],
                                 pp.cd[i],
                                 pp.cm[i],
                                 pp.cp_loc[i],
                                 pp.ct_loc[i],
                                 pp.element_thrust[i],
                                 pp.element_torque[i],
                                 pp.element_fy[i],
                                 pp.element_mz[i],
                                 pp.integral_fx[i],
                                 pp.integral_fy[i],
                                 pp.integral_mx[i],
                                 pp.integral_my[i],
                                 pp.integral_mz[i]});
        }

        fmt.addZone(zone);
    }

    return fmt;
}

// ─────────────────────────────────────────────────────────────────────────────
// Write — delegate to DataWriter (IFormatter + FileOutputTarget)
// ─────────────────────────────────────────────────────────────────────────────
bool TecplotSimulationExporter::Write(DataFormat const &fmt,
                                      std::string const &path) const
{
    namespace fs = std::filesystem;

    // ── Ensure output directory exists ────────────────────────────────────────
    fs::path file_path(path);
    fs::path dir = file_path.parent_path();
    if (!dir.empty() && !fs::exists(dir))
    {
        std::error_code ec;
        fs::create_directories(dir, ec);
        if (ec)
        {
            std::cerr << "Failed to create output directory '"
                      << dir << "': " << ec.message() << '\n';
            return false;
        }
        std::cout << "  -> Created output directory: " << dir << '\n';
    }

    // ── Delete existing file ──────────────────────────────────────────────────
    if (fs::exists(file_path))
    {
        std::error_code ec;
        fs::remove(file_path, ec);
        if (ec)
        {
            std::cerr << "Failed to delete existing file '"
                      << file_path << "': " << ec.message() << '\n';
            return false;
        }
        std::cout << "  -> Deleted existing file: " << file_path << '\n';
    }

    // ── Write ─────────────────────────────────────────────────────────────────
    auto data = std::make_shared<DataFormat>(fmt);
    auto output = std::make_shared<FileOutputTarget>(file_path);
    DataWriter writer(data, formatter_, output);
    return writer.write();
}
