#pragma once
/**
 * @file TurbineControlSettingsDataFileParser.h
 * @brief IDataFileParser adapter for TurbineControlSettingsParser.
 *
 * Bridges the IDataFileParser interface used by FilePathParser/ConfigurationParser
 * with the standalone TurbineControlSettingsParser so that the controller
 * settings file is loaded automatically as part of the Configuration pipeline —
 * exactly like blade_geometry or airfoil_geometry files.
 *
 * Registration key: "turbine_controller"
 * (derived from schema key "turbine_controller_file" by extractFileType())
 *
 * After parsing, the data is accessible via:
 *   config.getTurbineControlSettings()
 */
#include "IDataFileParser.h"
#include <vector>
#include <cmath>
#include <numbers>
#include <string>
#include <memory>
#include "TurbineControlSettingsParser.h"
#include "TurbineControlSettingsData.h"

// ─────────────────────────────────────────────────────────────────────────────
// TurbineControlSettingsStructuredData
//
// Wraps TurbineControlSettingsData in an IStructuredData shell so it can be
// stored in Configuration::structuredData.
// ─────────────────────────────────────────────────────────────────────────────
class TurbineControlSettingsStructuredData final : public IStructuredData
{
public:
    explicit TurbineControlSettingsStructuredData(TurbineControlSettingsData data)
        : data_(std::move(data))
    {
        applyAllUnitConversions();
    }

    std::string getTypeName() const override { return "TurbineControlSettings"; }
    size_t      getRowCount() const override { return data_.features.size(); }

    const TurbineControlSettingsData& data() const { return data_; }

private:
    TurbineControlSettingsData data_;

    // ── Unit conversions ──────────────────────────────────────────────────────
    //
    // Two passes are applied in order so compound units are handled correctly.
    //
    // Pass 1 — degrees → radians  (multiply by factor * π/180)
    //   Affects any feature whose unit string contains "deg".
    //   "deg" substrings in the unit are replaced with "rad".
    //
    // Pass 2 — kilowatts → watts
    //   "kW" as a numerator unit  → multiply values by 1000, replace "kW"→"W"
    //   "kW" as a denominator unit (preceded by '/')  → divide values by 1000,
    //   replace "/kW"→"/W"  (the quantity per watt becomes 1000× smaller).
    //   The factor field (numeric prefix) is also applied and reset to 1.0.
    //
    // Ordering matters: deg pass runs first so that a compound unit like
    // "0.01 deg/kW" becomes "rad/kW" before the kW pass divides by 1000,
    // yielding the final SI unit "rad/W".
    //
    // Concrete examples from TurbineControlSettings.dat:
    //   MinPthAngPwr    | 0.01 deg    :  -274  * 0.01 * π/180           → -0.04782  rad
    //   ALPHA_MIN_P_OPT | 0.01 deg    :     0  * 0.01 * π/180           →  0.0      rad
    //   D_ALPHA_DP_1    | 0.01 deg/kW :    30  * 0.01 * π/180 / 1000    →  5.236e-6 rad/W
    //   GnWMeas         | kW          :  values * 1.0 * 1000             →  W
    //   P_RATED         | kW          :  3500  * 1.0 * 1000              →  3 500 000 W
    //   Leistung_kW     | kW          :  values * 1.0 * 1000             →  W
    //   P_ALPHA_MIN_1   | kW          :  2600  * 1.0 * 1000              →  2 600 000 W
    //   P_SOLL_KL0      | kW          :  values * 1.0 * 1000             →  W
    //   P_SOLL_KL0_Stuetzst | rpm     :  values * 0.001 (factor only)    →  rpm
    //   DelMinPthAng    | 0.001 rpm   :  values * 0.001                  →  rpm
    //   N_SOLL_P_OPT    | 0.001 rpm   :  10800 * 0.001                  →  18.0    rpm
    //   SenkrAst_n_*    | 0.001 rpm   :  values * 0.001                  →  rpm

    // ── Pass 1 helpers ────────────────────────────────────────────────────────

    static bool unitContainsDeg(const std::string& unit)
    {
        return unit.find("deg") != std::string::npos;
    }

    static std::string replaceDegWithRad(const std::string& unit)
    {
        std::string result = unit;
        auto pos = result.find("deg");
        while (pos != std::string::npos)
        {
            result.replace(pos, 3, "rad");
            pos = result.find("deg", pos + 3);
        }
        return result;
    }

    void convertDegreeFeaturesToRadians()
    {
        constexpr double kDegToRad = std::numbers::pi / 180.0;

        for (auto& feature : data_.features)
        {
            if (!unitContainsDeg(feature.unit))
                continue;

            const double scale = feature.factor * kDegToRad;

            for (auto& entry : feature.entries)
                for (auto& v : entry.values)
                    v *= scale;

            feature.factor = 1.0;
            feature.unit   = replaceDegWithRad(feature.unit);
        }
    }

    // ── Pass 2 helpers ────────────────────────────────────────────────────────

    // Returns true if "kW" appears as a standalone numerator token, i.e.
    // not immediately preceded by '/'.
    static bool unitHasKwNumerator(const std::string& unit)
    {
        auto pos = unit.find("kW");
        if (pos == std::string::npos) return false;
        return (pos == 0 || unit[pos - 1] != '/');
    }

    // Returns true if "/kW" appears in the unit (kW in denominator position).
    static bool unitHasKwDenominator(const std::string& unit)
    {
        return unit.find("/kW") != std::string::npos;
    }

    static std::string replaceKwWithW(const std::string& unit)
    {
        // Replace every "kW" with "W" (works for both numerator and denominator)
        std::string result = unit;
        auto pos = result.find("kW");
        while (pos != std::string::npos)
        {
            result.replace(pos, 2, "W");
            pos = result.find("kW", pos + 1);
        }
        return result;
    }

    void convertKiloWattFeaturesToWatts()
    {
        for (auto& feature : data_.features)
        {
            const bool numKw  = unitHasKwNumerator(feature.unit);
            const bool denomKw = unitHasKwDenominator(feature.unit);

            if (!numKw && !denomKw)
                continue;

            // Start with the existing numeric factor (e.g. 1.0 after deg pass,
            // or the original factor if no deg conversion occurred).
            double scale = feature.factor;

            // kW in numerator   → values grow by 1000 (kW → W)
            if (numKw)   scale *= 1000.0;

            // kW in denominator → values shrink by 1000 (per-kW → per-W)
            if (denomKw) scale /= 1000.0;

            for (auto& entry : feature.entries)
                for (auto& v : entry.values)
                    v *= scale;

            feature.factor = 1.0;
            feature.unit   = replaceKwWithW(feature.unit);
        }
    }

    // ── Pass 3 — rpm: apply numeric factor only, keep rpm ────────────────────
    //
    // The controller layer (VariableSpeedController, ISimulationConfig) works
    // entirely in rpm.  We therefore do NOT convert rpm → rps.
    // We DO apply the numeric prefix factor (e.g. 0.001 from "0.001rpm") so
    // that the stored value is in plain rpm, and reset factor to 1.0.
    //
    //   P_SOLL_KL0_Stuetzst | rpm      : values * 1.0   → rpm (factor=1, no-op)
    //   DelMinPthAng        | 0.001rpm : values * 0.001 → rpm
    //   N_SOLL_P_OPT        | 0.001rpm : 10800  * 0.001 → 10.8 rpm
    //   SenkrAst_n_*        | 0.001rpm : values * 0.001 → rpm

    static bool unitContainsRpm(const std::string& unit)
    {
        return unit.find("rpm") != std::string::npos;
    }

    void applyRpmFactor()
    {
        for (auto& feature : data_.features)
        {
            if (!unitContainsRpm(feature.unit))
                continue;

            // Apply only the numeric prefix (e.g. 0.001); unit stays "rpm".
            if (feature.factor == 1.0)
                continue; // nothing to do

            for (auto& entry : feature.entries)
                for (auto& v : entry.values)
                    v *= feature.factor;

            feature.factor = 1.0;
            // unit stays e.g. "rpm" — no substring replacement needed
        }
    }

    void applyAllUnitConversions()
    {
        convertDegreeFeaturesToRadians(); // pass 1: deg   → rad
        convertKiloWattFeaturesToWatts(); // pass 2: kW    → W
        applyRpmFactor();                 // pass 3: bake numeric rpm prefix only
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// TurbineControlSettingsDataFileParser
//
// Implements IDataFileParser — FilePathParser calls parseFile() automatically
// when it encounters the "turbine_controller" file type.
// ─────────────────────────────────────────────────────────────────────────────
class TurbineControlSettingsDataFileParser final : public IDataFileParser
{
public:
    TurbineControlSettingsDataFileParser() = default;

    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) override
    {
        TurbineControlSettingsParser parser;
        auto ctrlData = parser.parse(filePath);
        return std::make_unique<TurbineControlSettingsStructuredData>(std::move(ctrlData));
    }

    std::vector<std::string> getSupportedExtensions() const override
    {
        return {".dat"};
    }
};
