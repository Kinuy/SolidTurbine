#pragma once
/**
 * @file ITurbSimReader.h
 * @brief Strategy interface for reading TurbSim wind-field data files.
 *
 * Open/Closed Principle: the Bladed/AeroDyn binary format (.wnd) and any
 * future format (HDF5, HAWC2 Mann, NetCDF) each implement this interface.
 * TurbSimManager depends on the abstraction; it never changes when a new
 * format is added.
 *
 * Single Responsibility: the reader owns only the file-parsing step.
 * It does not interpolate or index into the data.
 */
#include <string>
#include "TurbSimGrid.h"
#include "TurbSimVelocityData.h"
#include "TurbSimTimingInfo.h"

/**
 * @brief The result of parsing a TurbSim file, grouped as a value type.
 */
struct TurbSimFileData
{
    TurbSimGrid grid;
    TurbSimVelocityData velocity;
    TurbSimTimingInfo timing;
    double hub_velocity{0.0};

    // Provide a delegating constructor so timing can be built after the
    // grid parameters are known — timing needs grid_width_y.
    TurbSimFileData(TurbSimGrid g,
                    TurbSimVelocityData v,
                    TurbSimTimingInfo t,
                    double hub_vel)
        : grid(std::move(g)), velocity(std::move(v)), timing(std::move(t)), hub_velocity(hub_vel)
    {
    }
};

/**
 * @brief Pure interface for TurbSim file readers.
 */
class ITurbSimReader
{
public:
    virtual ~ITurbSimReader() = default;

    /**
     * @brief Parse a TurbSim file and return all data.
     * @param filename  Path to the wind-field file.
     * @throws std::runtime_error if the file cannot be opened or parsed.
     */
    virtual TurbSimFileData Read(std::string const &filename) const = 0;
};
