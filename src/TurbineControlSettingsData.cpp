/**
 * @file TurbineControlSettingsData.cpp
 * @brief Convenience query method implementations for TurbineControlSettingsData.
 */
#include "TurbineControlSettingsData.h"

// ─────────────────────────────────────────────────────────────────────────────
const ControlFeature* TurbineControlSettingsData::findFeature(
    const std::string& name) const
{
    for (auto const& f : features)
        if (f.name == name)
            return &f;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
std::vector<const ControlEntry*> TurbineControlSettingsData::findEntries(
    const std::string& feature_name,
    const std::string& scope,
    const std::string& mode) const
{
    const ControlFeature* feat = findFeature(feature_name);
    if (!feat) return {};

    std::vector<const ControlEntry*> result;
    for (auto const& e : feat->entries)
    {
        if (e.scope != scope)                  continue;
        if (!mode.empty() && e.mode != mode)   continue;
        result.push_back(&e);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
double TurbineControlSettingsData::getScalar(
    const std::string& feature_name,
    const std::string& scope,
    const std::string& mode) const
{
    auto entries = findEntries(feature_name, scope, mode);
    if (entries.empty())
        throw std::runtime_error(
            "TurbineControlSettingsData::getScalar: feature='" + feature_name +
            "' scope='" + scope + "' mode='" + mode + "' not found");

    if (entries.front()->values.empty())
        throw std::runtime_error(
            "TurbineControlSettingsData::getScalar: feature='" + feature_name +
            "' has no values");

    return entries.front()->values.front();
}

// ─────────────────────────────────────────────────────────────────────────────
const std::vector<double>& TurbineControlSettingsData::getVector(
    const std::string& feature_name,
    const std::string& scope,
    const std::string& mode) const
{
    auto entries = findEntries(feature_name, scope, mode);
    if (entries.empty())
        throw std::runtime_error(
            "TurbineControlSettingsData::getVector: feature='" + feature_name +
            "' scope='" + scope + "' mode='" + mode + "' not found");

    return entries.front()->values;
}
