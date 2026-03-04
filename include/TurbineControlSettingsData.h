#pragma once
/**
 * @file TurbineControlSettingsData.h
 * @brief Data containers for parsed turbine control settings.
 *
 * S — pure data, zero parsing or I/O logic.
 *
 * File format overview:
 * ─────────────────────
 * Each feature block consists of:
 *
 *   <FeatureName>  |  <FactorAndUnit>  |
 *       <scope> <turbine_id>{<mode>}  [$A|$T]  <value_or_list>
 *       ...
 *
 * scope      : "OMS" (applies only to that operation mode)
 *            | "WEA" (applies to all)
 * mode       : operating mode string from {…} suffix, e.g. "OM-1", "L0"
 * power_type : "$A" = mechanical power | "$T" = electrical power | "" = unspecified
 * values     : single double  |  explicit list [v1; v2; …]  |  range [start:step:end]
 * factor     : numeric prefix of the unit field, e.g. 0.01 from "0.01deg"
 */
#include <string>
#include <vector>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// ControlEntry — one data row beneath a feature header
// ─────────────────────────────────────────────────────────────────────────────
struct ControlEntry
{
    std::string         scope;        ///< "OMS" or "WEA"
    std::string         turbine_id;   ///< e.g. "TURBINE_NAME"
    std::string         mode;         ///< e.g. "OM-1", empty if absent
    std::string         power_type;   ///< "$A", "$T", or ""
    std::vector<double> values;       ///< parsed value list (size 1 for scalars)
};

// ─────────────────────────────────────────────────────────────────────────────
// ControlFeature — one complete feature block
// ─────────────────────────────────────────────────────────────────────────────
struct ControlFeature
{
    std::string                name;     ///< e.g. "Leistung_kW"
    double                     factor;   ///< numeric prefix of unit (1.0 if absent)
    std::string                unit;     ///< unit string, e.g. "kW", "deg", "rpm"
    std::vector<ControlEntry>  entries;  ///< one entry per data row
};

// ─────────────────────────────────────────────────────────────────────────────
// TurbineControlSettingsData — top-level container
// ─────────────────────────────────────────────────────────────────────────────
struct TurbineControlSettingsData
{
    std::vector<ControlFeature> features; ///< all features in file order

    // ── Convenience queries ───────────────────────────────────────────────────

    /**
     * @brief Find a feature by name.
     * @return Pointer to the feature, or nullptr if not found.
     */
    const ControlFeature* findFeature(const std::string& name) const;

    /**
     * @brief Find all entries for a feature filtered by scope and optional mode.
     * @param mode  Pass "" to return entries for all modes.
     */
    std::vector<const ControlEntry*> findEntries(
        const std::string& feature_name,
        const std::string& scope,
        const std::string& mode = "") const;

    /**
     * @brief Return the first scalar value for a feature/scope/mode.
     * @throws std::runtime_error if not found or values are empty.
     */
    double getScalar(const std::string& feature_name,
                     const std::string& scope,
                     const std::string& mode = "") const;

    /**
     * @brief Return the value vector for a feature/scope/mode.
     * @throws std::runtime_error if not found.
     */
    const std::vector<double>& getVector(const std::string& feature_name,
                                         const std::string& scope,
                                         const std::string& mode = "") const;
};
