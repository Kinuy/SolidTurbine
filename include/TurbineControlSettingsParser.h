#pragma once
/**
 * @file TurbineControlSettingsParser.h
 * @brief Concrete parser for the turbine control settings .dat format.
 *
 * SOLID compliance:
 *  S — responsible only for parsing; data lives in TurbineControlSettingsData.
 *  O — new formats: implement ITurbineControlSettingsParser.
 *  L — fully satisfies ITurbineControlSettingsParser.
 *  I — callers see only ITurbineControlSettingsParser.
 *  D — injection site (main.cpp) depends on the abstract interface.
 *
 * ── File format ──────────────────────────────────────────────────────────────
 *
 * Comment lines start with '#' and are ignored.
 * Empty lines are ignored.
 *
 * Feature header (contains '|'):
 *   <FeatureName>  |  <FactorAndUnit>  |
 *   FactorAndUnit examples: "0.01deg"  "0.001rpm"  "kW"  "-"
 *   A numeric prefix is extracted as factor; remainder is the unit.
 *
 * Data lines (indented, no '|'):
 *   <scope> <turbine_id>{<mode>}  [$A|$T]  <value_or_list>
 *
 *   scope      — "OMS" or "WEA"
 *   mode       — extracted from {…} suffix; empty if absent
 *   power_type — "$A" (mechanical) | "$T" (electrical) | absent
 *   value_or_list:
 *     - bare double:          "3500"  or  "-274"
 *     - explicit list:        "[0; 45; 100; 163]"
 *     - range shorthand:      "[5.0:0.2:12.8]"
 *
 * Inline comments start with '%' outside of '[…]' and are stripped.
 * Stray extra '}' characters (typos in source file) are tolerated.
 */
#include "ITurbineControlSettingsParser.h"
#include <string>
#include <vector>
#include <utility>

class TurbineControlSettingsParser final : public ITurbineControlSettingsParser
{
public:
    TurbineControlSettingsParser() = default;

    TurbineControlSettingsData parse(const std::string& file_path) const override;

private:
    // ── Line classification ───────────────────────────────────────────────────
    static bool isComment(const std::string& line);
    static bool isFeatureHeader(const std::string& line);

    // ── Feature header ────────────────────────────────────────────────────────
    static ControlFeature parseFeatureHeader(const std::string& line);

    /**
     * @brief Split "0.01deg" → {0.01, "deg"}.
     *        Returns {1.0, raw} when no numeric prefix is found.
     */
    static std::pair<double, std::string> parseFactorAndUnit(const std::string& raw);

    // ── Data line ─────────────────────────────────────────────────────────────
    static ControlEntry parseDataLine(const std::string& line);

    /**
     * @brief Split "TURBINE_NAME{OM-1}" → {"TURBINE_NAME", "OM-1"}.
     *        Returns mode="" when no braces are present.
     *        Tolerates stray extra '}' characters.
     */
    static std::pair<std::string, std::string>
        parseTurbineIdAndMode(const std::string& token);

    /// Strip inline comment starting at an unquoted '%' (outside '[…]').
    static std::string stripInlineComment(const std::string& s);

    /**
     * @brief Reassemble whitespace-split tokens, merging '[…]' into one token.
     *        Handles lists that contain spaces after ';'.
     */
    static std::vector<std::string> tokenise(const std::string& line);

    // ── Value parsing ─────────────────────────────────────────────────────────
    static std::vector<double> parseValues(const std::string& token);

    /// Expand "[start:step:end]" range.
    static std::vector<double> expandRange(const std::string& inner);

    /// Parse "[v1; v2; v3]" explicit list.
    static std::vector<double> parseList(const std::string& inner);
};
