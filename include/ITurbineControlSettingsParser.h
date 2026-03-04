#pragma once
/**
 * @file ITurbineControlSettingsParser.h
 * @brief Abstract interface for turbine control settings parsers.
 *
 * OCP / DIP: callers depend on this abstraction only.
 * New file formats are added by deriving from this interface without
 * touching any existing code.
 */
#include "TurbineControlSettingsData.h"
#include <string>

class ITurbineControlSettingsParser
{
public:
    virtual ~ITurbineControlSettingsParser() = default;

    /**
     * @brief Parse a control settings file.
     * @param file_path  Path to the .dat file.
     * @return           Fully populated TurbineControlSettingsData.
     * @throws std::runtime_error on I/O or parse errors.
     */
    virtual TurbineControlSettingsData parse(const std::string& file_path) const = 0;
};
