/**
 * @file TecplotNoiseExporter.cpp
 * @brief Tecplot noise exporter — single operating point and full power curve.
 */
#include "TecplotNoiseExporter.h"
#include "DataWriter.h"
#include "FileOutputTarget.h"

#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
TecplotNoiseExporter::TecplotNoiseExporter(std::shared_ptr<IFormatter> formatter)
    : formatter_(std::move(formatter))
{}

// ─────────────────────────────────────────────────────────────────────────────
// INoiseResultsExporter
// ─────────────────────────────────────────────────────────────────────────────
bool TecplotNoiseExporter::ExportBladeNoise(
    BladeNoiseResult const &result,
    std::string      const &output_path) const
{
    return Write(BuildSinglePointFormat(result), output_path);
}

bool TecplotNoiseExporter::ExportPowerCurveNoise(
    std::vector<BladeNoiseResult> const &results,
    std::string                   const &output_path) const
{
    return Write(BuildPowerCurveFormat(results), output_path);
}

// ─────────────────────────────────────────────────────────────────────────────
// SinglePointPrecisions
// Column order: radius(3), chord(3), v_loc(2), alpha_eff(2), OASPL(2),
//               SPL[N bands](2)
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<int> SinglePointPrecisions(int n_bands)
{
    std::vector<int> p = {3, 3, 2, 2, 2};   // fixed columns
    p.insert(p.end(), n_bands, 2);           // one entry per 1/3-oct band
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildSinglePointFormat
// One zone per noise source, rows = blade sections.
// Variables: radius, chord, v_loc, alpha, OASPL, SPL[34 bands].
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotNoiseExporter::BuildSinglePointFormat(
    BladeNoiseResult const &result)
{
    DataFormat fmt("BladeNoise v_inf=" + std::to_string(result.vinf) + "m_s");

    // Fixed columns + one column per 1/3-octave band
    std::vector<std::string> vars = {
        "radius_[m]", "chord_[m]", "v_loc_[m/s]", "alpha_eff_[deg]", "OASPL_[dB]"
    };
    for (double f : result.frequencies)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << f << "_Hz_[dB]";
        vars.push_back(ss.str());
    }
    fmt.setVariables(vars);

    if (result.sections.empty()) return fmt;

    const int I = static_cast<int>(result.sections.size());
    const int n_bands = static_cast<int>(result.frequencies.size());
    const std::vector<int> precisions = SinglePointPrecisions(n_bands);

    auto addZone = [&](std::string const &zone_title,
                       SectionNoiseSpectrum SectionNoiseResult::*src)
    {
        DataZone zone(zone_title, I);
        zone.columnPrecisions = precisions;
        for (auto const &sec : result.sections)
        {
            SectionNoiseSpectrum const &sp = sec.*src;
            std::vector<double> row = {
                sec.radius, sec.chord, sec.velocity, sec.alpha_deg, sp.oaspl
            };
            for (double v : sp.spl) row.push_back(v);
            while (static_cast<int>(row.size()) < static_cast<int>(vars.size()))
                row.push_back(-100.0);
            zone.data.push_back(row);
        }
        fmt.addZone(zone);
    };

    addZone("TBL_pressure_side",   &SectionNoiseResult::tbl_pressure_side);
    addZone("TBL_suction_side",    &SectionNoiseResult::tbl_suction_side);
    addZone("Separation",          &SectionNoiseResult::separation);
    addZone("LBL_vortex_shedding", &SectionNoiseResult::laminar_vortex);
    addZone("Bluntness",           &SectionNoiseResult::bluntness);
    addZone("Turbulent_inflow",    &SectionNoiseResult::turbulent_inflow);
    addZone("Total",               &SectionNoiseResult::total);

    return fmt;
}

// ─────────────────────────────────────────────────────────────────────────────
// PowerCurvePrecisions
// Column order: v_inf(2), radius(3), chord(3), v_loc(2), Re(0), Mach(3),
//               alpha(2), OASPL_total(2), OASPL_TBL_p(2), OASPL_TBL_s(2),
//               OASPL_sep(2), OASPL_LBL(2), OASPL_blunt(2), OASPL_TI(2),
//               LWA_total(2), SPL_total[N bands](2)
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<int> PowerCurvePrecisions(int n_bands)
{
    std::vector<int> p = {
        2,  // v_inf
        3,  // radius
        3,  // chord
        2,  // v_loc
        0,  // Re
        3,  // Mach
        2,  // alpha_eff
        2,  // OASPL_total
        2,  // OASPL_TBL_pressure
        2,  // OASPL_TBL_suction
        2,  // OASPL_separation
        2,  // OASPL_LBL_VS
        2,  // OASPL_bluntness
        2,  // OASPL_turbulent_inflow
        2,  // LWA_total
    };
    p.insert(p.end(), n_bands, 2);   // SPL spectrum columns
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// PowerCurveVariables
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::string> TecplotNoiseExporter::PowerCurveVariables(
    std::vector<double> const &frequencies)
{
    std::vector<std::string> vars = {
        "v_inf_[m/s]",
        "radius_[m]",
        "chord_[m]",
        "v_loc_[m/s]",
        "Re_[-]",
        "Mach_[-]",
        "alpha_eff_[deg]",
        "OASPL_total_[dB]",
        "OASPL_TBL_pressure_[dB]",
        "OASPL_TBL_suction_[dB]",
        "OASPL_separation_[dB]",
        "OASPL_LBL_VS_[dB]",
        "OASPL_bluntness_[dB]",
        "OASPL_turbulent_inflow_[dB]",
        "LWA_total_[dB_re_1pW]",        // sound power level, section contribution
    };
    // One SPL column per 1/3-octave band for the total source
    for (double f : frequencies)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << "SPL_total_" << f << "Hz_[dB]";
        vars.push_back(ss.str());
    }
    return vars;
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeLWA
// Sound power level [dB re 1 pW] for one noise source, summed over span.
// LW_section = OASPL + 10·log10(chord × span)   [radiating area proxy]
// LWA_total  = 10·log10( Σ_i 10^(LW_section_i / 10) )
// ─────────────────────────────────────────────────────────────────────────────
double TecplotNoiseExporter::ComputeLWA(
    BladeNoiseResult const &result,
    SectionNoiseSpectrum SectionNoiseResult::*src)
{
    double sum_linear = 0.0;
    for (auto const &sec : result.sections)
    {
        if (!sec.converged) continue;
        SectionNoiseSpectrum const &sp = sec.*src;
        if (sp.oaspl <= -99.0) continue;
        // Radiating area per section: chord × span.
        const double area_proxy = (sec.span > 0.0)
                                ? sec.chord * sec.span
                                : sec.chord; // fallback: per-unit-span
        const double lw_sec = sp.oaspl + 10.0 * std::log10(area_proxy + 1e-30);
        sum_linear += std::pow(10.0, lw_sec / 10.0);
    }
    if (sum_linear <= 0.0) return -100.0;
    return 10.0 * std::log10(sum_linear);
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildOperatingPointZone
// One row per blade section; all noise source OASPLs + total SPL spectrum.
// ─────────────────────────────────────────────────────────────────────────────
DataZone TecplotNoiseExporter::BuildOperatingPointZone(
    BladeNoiseResult const &result)
{
    const int n_sec = static_cast<int>(result.sections.size());
    std::ostringstream zone_title;
    zone_title << std::fixed << std::setprecision(2)
               << "v_inf=" << result.vinf << "m_s";
    DataZone zone(zone_title.str(), n_sec);

    // Per-column precision — derive band count from first section's total SPL
    const int n_bands = result.sections.empty()
                      ? 0
                      : static_cast<int>(result.sections.front().total.spl.size());
    zone.columnPrecisions = PowerCurvePrecisions(n_bands);

    // Pre-compute LWA for the total source at this operating point
    // (same value for every row in the zone — it is an integrated quantity)
    const double lwa_total = ComputeLWA(result, &SectionNoiseResult::total);

    for (auto const &sec : result.sections)
    {
        std::vector<double> row = {
            result.vinf,
            sec.radius,
            sec.chord,
            sec.velocity,
            sec.reynolds,
            sec.mach,
            sec.alpha_deg,
            sec.total.oaspl,
            sec.tbl_pressure_side.oaspl,
            sec.tbl_suction_side.oaspl,
            sec.separation.oaspl,
            sec.laminar_vortex.oaspl,
            sec.bluntness.oaspl,
            sec.turbulent_inflow.oaspl,
            lwa_total,
        };
        // SPL spectrum of total source
        for (double v : sec.total.spl) row.push_back(v);
        zone.data.push_back(row);
    }
    return zone;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildPowerCurveFormat
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotNoiseExporter::BuildPowerCurveFormat(
    std::vector<BladeNoiseResult> const &results)
{
    DataFormat fmt("PowerCurveNoise");

    // Derive frequency list from first valid result
    std::vector<double> frequencies;
    for (auto const &r : results)
        if (!r.frequencies.empty()) { frequencies = r.frequencies; break; }

    fmt.setVariables(PowerCurveVariables(frequencies));

    for (auto const &r : results)
    {
        if (r.sections.empty()) continue;
        fmt.addZone(BuildOperatingPointZone(r));
    }
    return fmt;
}

// ─────────────────────────────────────────────────────────────────────────────
// Write
// ─────────────────────────────────────────────────────────────────────────────
bool TecplotNoiseExporter::Write(DataFormat const &fmt,
                                 std::string const &path) const
{
    const fs::path file_path(path);
    if (file_path.has_parent_path())
        fs::create_directories(file_path.parent_path());

    if (fs::exists(file_path))
    {
        std::error_code ec;
        fs::remove(file_path, ec);
        if (ec)
        {
            std::cerr << "TecplotNoiseExporter: cannot remove '"
                      << path << "': " << ec.message() << '\n';
            return false;
        }
    }

    auto data   = std::make_shared<DataFormat>(fmt);
    auto output = std::make_shared<FileOutputTarget>(path);
    DataWriter writer(data, formatter_, output);
    return writer.write();
}

// ─────────────────────────────────────────────────────────────────────────────
// ExportRotorNoise
// ─────────────────────────────────────────────────────────────────────────────
bool TecplotNoiseExporter::ExportRotorNoise(
    std::vector<RotorNoiseResult> const &results,
    std::string                   const &output_path) const
{
    return Write(BuildRotorNoiseFormat(results), output_path);
}

// ─────────────────────────────────────────────────────────────────────────────
// RotorNoiseVariables
// Columns: v_inf, observer_dist, OASPL, OASPL_A, LW, LWA,
//          SPL_<f>Hz, SPLA_<f>Hz   (one pair per 1/3-oct band)
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::string> TecplotNoiseExporter::RotorNoiseVariables(
    std::vector<double> const &frequencies)
{
    std::vector<std::string> vars = {
        "v_inf_[m/s]",
        "observer_dist_[m]",
        "OASPL_[dB]",
        "OASPL_A_[dB(A)]",
        "LW_[dB_re_1pW]",
        "LWA_[dB(A)_re_1pW]",
    };
    for (double f : frequencies)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << "SPL_" << f << "Hz_[dB]";
        vars.push_back(ss.str());
    }
    for (double f : frequencies)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << "SPLA_" << f << "Hz_[dB(A)]";
        vars.push_back(ss.str());
    }
    return vars;
}

// ─────────────────────────────────────────────────────────────────────────────
// RotorNoisePrecisions
// v_inf(2), observer_dist(2), OASPL(2), OASPL_A(2), LW(2), LWA(2),
// SPL bands(2), SPLA bands(2)
// ─────────────────────────────────────────────────────────────────────────────
std::vector<int> TecplotNoiseExporter::RotorNoisePrecisions(int n_bands)
{
    std::vector<int> p = {2, 2, 2, 2, 2, 2};       // scalar columns
    p.insert(p.end(), n_bands, 2);                  // SPL spectrum
    p.insert(p.end(), n_bands, 2);                  // SPLA spectrum
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildRotorNoiseSourceZone
// One zone = one noise source; each row = one operating point.
// ─────────────────────────────────────────────────────────────────────────────
DataZone TecplotNoiseExporter::BuildRotorNoiseSourceZone(
    std::vector<RotorNoiseResult>  const &results,
    std::string                    const &zone_title,
    RotorNoiseSourceResult RotorNoiseResult::*src)
{
    const int n_pts = static_cast<int>(results.size());
    DataZone zone(zone_title, n_pts);

    // Derive band count from first non-empty result
    int n_bands = 0;
    for (auto const &r : results)
        if (!(r.*src).spl_spectrum.empty())
        {
            n_bands = static_cast<int>((r.*src).spl_spectrum.size());
            break;
        }

    zone.columnPrecisions = RotorNoisePrecisions(n_bands);

    for (auto const &r : results)
    {
        RotorNoiseSourceResult const &s = r.*src;
        std::vector<double> row = {
            r.vinf,
            r.observer_distance,
            s.oaspl,
            s.oasplA,
            s.lw,
            s.lwA,
        };
        // SPL spectrum
        for (double v : s.spl_spectrum) row.push_back(v);
        while (static_cast<int>(row.size()) < 6 + n_bands)
            row.push_back(-100.0);
        // SPLA spectrum
        for (double v : s.splA_spectrum) row.push_back(v);
        while (static_cast<int>(row.size()) < 6 + 2 * n_bands)
            row.push_back(-100.0);

        zone.data.push_back(row);
    }
    return zone;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildRotorNoiseFormat
// One zone per noise source — 7 zones total (matches single-point format).
// ─────────────────────────────────────────────────────────────────────────────
DataFormat TecplotNoiseExporter::BuildRotorNoiseFormat(
    std::vector<RotorNoiseResult> const &results)
{
    DataFormat fmt("RotorNoise");

    // Derive frequency list from first non-empty result
    std::vector<double> frequencies;
    for (auto const &r : results)
        if (!r.frequencies.empty()) { frequencies = r.frequencies; break; }

    fmt.setVariables(RotorNoiseVariables(frequencies));

    if (results.empty()) return fmt;

    auto addZone = [&](std::string const &title,
                       RotorNoiseSourceResult RotorNoiseResult::*src)
    {
        fmt.addZone(BuildRotorNoiseSourceZone(results, title, src));
    };

    addZone("TBL_pressure_side",   &RotorNoiseResult::tbl_pressure_side);
    addZone("TBL_suction_side",    &RotorNoiseResult::tbl_suction_side);
    addZone("Separation",          &RotorNoiseResult::separation);
    addZone("LBL_vortex_shedding", &RotorNoiseResult::laminar_vortex);
    addZone("Bluntness",           &RotorNoiseResult::bluntness);
    addZone("Turbulent_inflow",    &RotorNoiseResult::turbulent_inflow);
    addZone("Total",               &RotorNoiseResult::total);

    return fmt;
}
