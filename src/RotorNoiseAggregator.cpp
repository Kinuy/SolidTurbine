/**
 * @file RotorNoiseAggregator.cpp
 * @brief Rotor-level noise aggregation — see header for algorithm description.
 */
#include "RotorNoiseAggregator.h"

#include <cmath>
#include <numbers>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

RotorNoiseAggregator::RotorNoiseAggregator(int    n_blades,
                                           double hub_height,
                                           double ground_distance,
                                           double overhang)
    : n_blades_(n_blades)
{
    if (n_blades_ < 1)
        throw std::invalid_argument("RotorNoiseAggregator: n_blades must be >= 1");

    // IEC 61400-11: ground-level microphone (mic_height = 0).
    // Matches Python BladeNoise: observer_distance = sqrt(hub_height² + (ground_distance + overhang)²)
    const double dh = hub_height;                    // vertical leg
    const double dx = ground_distance + overhang;    // horizontal leg
    observer_distance_ = std::sqrt(dh * dh + dx * dx);

    if (observer_distance_ <= 0.0)
        throw std::invalid_argument(
            "RotorNoiseAggregator: observer distance must be > 0");
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

RotorNoiseResult RotorNoiseAggregator::Aggregate(
    BladeNoiseResult const &blade_result) const
{
    RotorNoiseResult result;
    result.vinf              = blade_result.vinf;
    result.observer_distance = observer_distance_;
    result.n_blades          = n_blades_;
    result.frequencies       = blade_result.frequencies;

    if (blade_result.sections.empty() || blade_result.frequencies.empty())
        return result;

    const std::vector<double> a_weights =
        ComputeAWeights(blade_result.frequencies);

    const double As = GeometricAttenuation(observer_distance_);

    auto agg = [&](SectionNoiseSpectrum SectionNoiseResult::*src)
    {
        return AggregateSource(blade_result.sections, src,
                               a_weights, As, n_blades_);
    };

    result.tbl_pressure_side = agg(&SectionNoiseResult::tbl_pressure_side);
    result.tbl_suction_side  = agg(&SectionNoiseResult::tbl_suction_side);
    result.separation        = agg(&SectionNoiseResult::separation);
    result.laminar_vortex    = agg(&SectionNoiseResult::laminar_vortex);
    result.bluntness         = agg(&SectionNoiseResult::bluntness);
    result.turbulent_inflow  = agg(&SectionNoiseResult::turbulent_inflow);
    result.total             = agg(&SectionNoiseResult::total);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Static helpers
// ─────────────────────────────────────────────────────────────────────────────

std::vector<double> RotorNoiseAggregator::ComputeAWeights(
    std::vector<double> const &frequencies)
{
    // Standard A-weighting formula (matches Python calc_weight_A):
    //   RA(f) = 12200² · f⁴ /
    //           [ (f²+20.6²) · sqrt((f²+107.7²)(f²+737.9²)) · (f²+12200²) ]
    //   A(f)  = 2 + 20·log10(RA(f))   [dB]
    constexpr double f1  = 20.6;
    constexpr double f2  = 107.7;
    constexpr double f3  = 737.9;
    constexpr double f4  = 12200.0;
    constexpr double f4sq = f4 * f4;

    std::vector<double> aw;
    aw.reserve(frequencies.size());

    for (double f : frequencies)
    {
        const double f2_  = f * f;
        const double RA   = f4sq * f2_ * f2_
                          / ((f2_ + f1 * f1)
                             * std::sqrt((f2_ + f2 * f2) * (f2_ + f3 * f3))
                             * (f2_ + f4sq));
        aw.push_back(2.0 + 20.0 * std::log10(RA));
    }
    return aw;
}

double RotorNoiseAggregator::GeometricAttenuation(double observer_distance)
{
    // Free-field (Q=1, infinite room constant R → 4/R → 0):
    //   As = 10·log10( 1 / (4·π·d²) )
    // This is a negative number — subtracting it from SPL gives LW.
    return 10.0 * std::log10(
        1.0 / (4.0 * std::numbers::pi * observer_distance * observer_distance));
}

double RotorNoiseAggregator::EnergySum(std::vector<double> const &levels)
{
    // 10·log10( Σ_i 10^(L_i/10) )
    // Levels below -99 dB are treated as inactive and skipped.
    double sum = 0.0;
    for (double L : levels)
        if (L > -99.0)
            sum += std::pow(10.0, L / 10.0);

    if (sum <= 0.0) return -100.0;
    return 10.0 * std::log10(sum);
}

RotorNoiseSourceResult RotorNoiseAggregator::AggregateSource(
    std::vector<SectionNoiseResult> const &sections,
    SectionNoiseSpectrum SectionNoiseResult::*src,
    std::vector<double>  const &a_weights,
    double               As,
    int                  n_blades)
{
    RotorNoiseSourceResult out;
    if (sections.empty()) return out;

    const std::size_t n_bands = (sections.front().*src).spl.size();
    if (n_bands == 0) return out;

    // ── Step 1: energy-sum per frequency band over all blade sections ─────────
    // spl_band[k] = 10·log10( Σ_sections  10^(spl_section[k]/10) )
    out.spl_spectrum.resize(n_bands, -100.0);
    out.splA_spectrum.resize(n_bands, -100.0);

    for (std::size_t k = 0; k < n_bands; ++k)
    {
        double sum_lin = 0.0;
        for (auto const &sec : sections)
        {
            if (!sec.converged) continue;
            const SectionNoiseSpectrum &sp = sec.*src;
            if (k < sp.spl.size() && sp.spl[k] > -99.0)
                sum_lin += std::pow(10.0, sp.spl[k] / 10.0);
        }
        if (sum_lin > 0.0)
        {
            out.spl_spectrum[k] = 10.0 * std::log10(sum_lin);
            // A-weight the blade SPL spectrum
            const double aw = (k < a_weights.size()) ? a_weights[k] : 0.0;
            out.splA_spectrum[k] = out.spl_spectrum[k] + aw;
        }
    }

    // ── Step 2: OASPL = energy-sum of SPL spectrum over all bands ─────────────
    out.oaspl  = EnergySum(out.spl_spectrum);
    out.oasplA = EnergySum(out.splA_spectrum);

    // ── Step 3: LW = SPL - As  (As is negative, so LW > SPL) ─────────────────
    //   LW_blade = SPL_blade − As
    //   LWA_rotor = LWA_blade + 10·log10(n_blades)
    //
    // Compute at spectrum level first (needed if callers want per-band LW later).
    const double n_blades_dB = 10.0 * std::log10(static_cast<double>(n_blades));

    std::vector<double> lw_spectrum(n_bands, -100.0);
    std::vector<double> lwA_spectrum(n_bands, -100.0);
    for (std::size_t k = 0; k < n_bands; ++k)
    {
        if (out.spl_spectrum[k] > -99.0)
            lw_spectrum[k] = out.spl_spectrum[k] - As;
        if (out.splA_spectrum[k] > -99.0)
            lwA_spectrum[k] = out.splA_spectrum[k] - As + n_blades_dB;
    }

    // OASPL LW: energy-sum of per-band LW
    out.lw  = EnergySum(lw_spectrum);
    out.lwA = EnergySum(lwA_spectrum);

    return out;
}
