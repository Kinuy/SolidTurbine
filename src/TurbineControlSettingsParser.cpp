/**
 * @file TurbineControlSettingsParser.cpp
 * @brief Implementation of TurbineControlSettingsParser.
 *
 * Single-pass strategy:
 *   1. Skip comment ('#') and empty lines.
 *   2. Line contains '|'  → feature header: create new ControlFeature.
 *   3. Otherwise          → data line: parse as ControlEntry, append to
 *                           the active ControlFeature.
 *   4. Parse errors on individual lines are reported to stderr and skipped
 *      so that a single malformed line does not abort the whole file.
 */
#include "TurbineControlSettingsParser.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// ═════════════════════════════════════════════════════════════════════════════
// File-local utilities
// ═════════════════════════════════════════════════════════════════════════════
namespace {

std::string trim(const std::string& s)
{
    const std::string ws = " \t\r\n";
    auto first = s.find_first_not_of(ws);
    if (first == std::string::npos) return {};
    auto last = s.find_last_not_of(ws);
    return s.substr(first, last - first + 1);
}

std::vector<std::string> splitOn(const std::string& s, char delim)
{
    std::vector<std::string> out;
    std::istringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, delim))
        out.push_back(tok);
    return out;
}

std::vector<std::string> splitWS(const std::string& s)
{
    std::vector<std::string> out;
    std::istringstream ss(s);
    std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}

} // namespace

// ═════════════════════════════════════════════════════════════════════════════
// Public: parse()
// ═════════════════════════════════════════════════════════════════════════════
TurbineControlSettingsData TurbineControlSettingsParser::parse(
    const std::string& file_path) const
{
    std::ifstream file(file_path);
    if (!file.is_open())
        throw std::runtime_error(
            "TurbineControlSettingsParser: cannot open '" + file_path + "'");

    TurbineControlSettingsData data;
    ControlFeature* activeFeature = nullptr;
    int line_number = 0;

    std::string line;
    while (std::getline(file, line))
    {
        ++line_number;
        std::string t = trim(line);
        if (t.empty() || isComment(t)) continue;

        try
        {
            if (isFeatureHeader(t))
            {
                data.features.push_back(parseFeatureHeader(t));
                activeFeature = &data.features.back();
            }
            else
            {
                if (!activeFeature)
                {
                    std::cerr << "[TurbineControlSettingsParser] line "
                              << line_number
                              << ": data line before any feature header — skipped\n";
                    continue;
                }
                activeFeature->entries.push_back(parseDataLine(t));
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[TurbineControlSettingsParser] line " << line_number
                      << ": " << e.what() << " — skipped\n";
        }
    }

    std::cout << "[TurbineControlSettingsParser] parsed "
              << data.features.size()
              << " features from '" << file_path << "'\n";
    return data;
}

// ═════════════════════════════════════════════════════════════════════════════
// Line classification
// ═════════════════════════════════════════════════════════════════════════════
bool TurbineControlSettingsParser::isComment(const std::string& line)
{
    return !line.empty() && line.front() == '#';
}

bool TurbineControlSettingsParser::isFeatureHeader(const std::string& line)
{
    return line.find('|') != std::string::npos;
}

// ═════════════════════════════════════════════════════════════════════════════
// Feature header parsing
// ═════════════════════════════════════════════════════════════════════════════
ControlFeature TurbineControlSettingsParser::parseFeatureHeader(
    const std::string& line)
{
    // Format:  FeatureName  |  FactorAndUnit  |
    auto parts = splitOn(line, '|');
    if (parts.size() < 2)
        throw std::runtime_error(
            "feature header missing '|' separator: '" + line + "'");

    ControlFeature feat;
    feat.name = trim(parts[0]);

    auto [factor, unit] = parseFactorAndUnit(trim(parts[1]));
    feat.factor = factor;
    feat.unit   = unit;

    return feat;
}

std::pair<double, std::string> TurbineControlSettingsParser::parseFactorAndUnit(
    const std::string& raw)
{
    if (raw.empty() || raw == "-")
        return {1.0, raw};

    // Scan a numeric prefix (optional sign, digits, optional decimal point).
    // Examples: "0.01deg" → 0.01, "deg"
    //           "0.001rpm" → 0.001, "rpm"
    //           "kW"  → 1.0, "kW"   (no numeric prefix)
    std::size_t pos = 0;
    if (pos < raw.size() && (raw[pos] == '-' || raw[pos] == '+'))
        ++pos;

    std::size_t digitStart = pos;
    while (pos < raw.size() && (std::isdigit(static_cast<unsigned char>(raw[pos]))
                                 || raw[pos] == '.'))
        ++pos;

    bool hasNumericPrefix = (pos > digitStart);
    if (!hasNumericPrefix)
        return {1.0, raw};

    double factor = 1.0;
    try {
        factor = std::stod(raw.substr(0, pos));
    } catch (...) {
        return {1.0, raw};
    }

    std::string unit = trim(raw.substr(pos));
    return {factor, unit};
}

// ═════════════════════════════════════════════════════════════════════════════
// Data line parsing
// ═════════════════════════════════════════════════════════════════════════════
ControlEntry TurbineControlSettingsParser::parseDataLine(const std::string& line)
{
    std::string clean = stripInlineComment(line);
    auto tokens = tokenise(clean);

    if (tokens.size() < 2)
        throw std::runtime_error(
            "data line has fewer than 2 tokens: '" + line + "'");

    ControlEntry entry;

    // Token 0: scope — "OMS" or "WEA"
    entry.scope = tokens[0];

    // Token 1: turbine_id{mode}
    auto [tid, mode] = parseTurbineIdAndMode(tokens[1]);
    entry.turbine_id = tid;
    entry.mode       = mode;

    // Remaining tokens: optional $A/$T, then value / list
    std::size_t i = 2;
    if (i < tokens.size() &&
        (tokens[i] == "$A" || tokens[i] == "$T"))
    {
        entry.power_type = tokens[i];
        ++i;
    }

    if (i < tokens.size())
        entry.values = parseValues(tokens[i]);

    return entry;
}

std::pair<std::string, std::string>
TurbineControlSettingsParser::parseTurbineIdAndMode(const std::string& token)
{
    // Format: "TURBINE_NAME{OM-1}" or bare "TURBINE_NAME"
    // Tolerate stray extra '}' as in "TURBINE_NAME{OM-4}}" from the source file.
    auto brace = token.find('{');
    if (brace == std::string::npos)
        return {token, ""};

    std::string id   = token.substr(0, brace);
    std::string rest = token.substr(brace + 1);

    // Strip everything from the first '}' onward
    auto close = rest.find('}');
    std::string mode = (close != std::string::npos) ? rest.substr(0, close) : rest;

    return {id, mode};
}

std::string TurbineControlSettingsParser::stripInlineComment(const std::string& s)
{
    // '%' outside '[…]' starts an inline comment.
    int depth = 0;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if      (s[i] == '[') ++depth;
        else if (s[i] == ']') --depth;
        else if (s[i] == '%' && depth == 0)
            return trim(s.substr(0, i));
    }
    return s;
}

std::vector<std::string> TurbineControlSettingsParser::tokenise(
    const std::string& line)
{
    // Split on whitespace first.
    auto raw = splitWS(line);

    // Merge tokens that form a single '[…]' list broken across spaces.
    // e.g.  "[0;" "45;" "100]"  →  "[0; 45; 100]"
    std::vector<std::string> tokens;
    for (std::size_t i = 0; i < raw.size(); ++i)
    {
        std::string t = raw[i];
        bool opensB = t.find('[') != std::string::npos;
        bool closesB = t.find(']') != std::string::npos;

        if (opensB && !closesB)
        {
            // Accumulate until we find the closing ']'
            while (i + 1 < raw.size())
            {
                ++i;
                t += ' ' + raw[i];
                if (raw[i].find(']') != std::string::npos)
                    break;
            }
        }
        tokens.push_back(t);
    }
    return tokens;
}

// ═════════════════════════════════════════════════════════════════════════════
// Value parsing
// ═════════════════════════════════════════════════════════════════════════════
std::vector<double> TurbineControlSettingsParser::parseValues(
    const std::string& token)
{
    std::string t = trim(token);
    if (t.empty()) return {};

    if (t.front() == '[')
    {
        // Strip surrounding '[' and ']'
        auto close = t.rfind(']');
        std::string inner = (close != std::string::npos)
                            ? trim(t.substr(1, close - 1))
                            : trim(t.substr(1));

        // Range: contains ':' but not ';'
        if (inner.find(':') != std::string::npos &&
            inner.find(';') == std::string::npos)
            return expandRange(inner);

        return parseList(inner);
    }

    // Single value (may be negative, e.g. "-274")
    try {
        return {std::stod(t)};
    } catch (...) {
        throw std::runtime_error("cannot parse value token: '" + t + "'");
    }
}

std::vector<double> TurbineControlSettingsParser::expandRange(
    const std::string& inner)
{
    // Format: "start:step:end"
    auto parts = splitOn(inner, ':');
    if (parts.size() != 3)
        throw std::runtime_error(
            "range must have exactly 3 ':'-parts, got: '" + inner + "'");

    double start = std::stod(trim(parts[0]));
    double step  = std::stod(trim(parts[1]));
    double end   = std::stod(trim(parts[2]));

    if (std::abs(step) < 1e-15)
        throw std::runtime_error("range step must not be zero");

    std::vector<double> result;
    // Use small epsilon to include floating-point endpoint correctly
    double eps = std::abs(step) * 1e-9;
    for (double v = start;
         step > 0 ? v <= end + eps : v >= end - eps;
         v += step)
        result.push_back(v);

    return result;
}

std::vector<double> TurbineControlSettingsParser::parseList(
    const std::string& inner)
{
    // Format: "v1; v2; v3; ..."
    auto parts = splitOn(inner, ';');
    std::vector<double> result;
    result.reserve(parts.size());
    for (auto const& p : parts)
    {
        std::string t = trim(p);
        if (t.empty()) continue;
        try {
            result.push_back(std::stod(t));
        } catch (...) {
            throw std::runtime_error(
                "cannot parse list element: '" + t + "'");
        }
    }
    return result;
}
