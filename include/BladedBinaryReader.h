#pragma once
/**
 * @file BladedBinaryReader.h
 * @brief Reads the Bladed/AeroDyn full-field binary (.wnd) TurbSim format.
 *
 * Single Responsibility: owns only the binary parsing of the .wnd format
 * (TurbSim manual Appendix E).  No interpolation, no grid indexing.
 *
 * Open/Closed: implementing a new format means creating a new class, not
 * modifying this one.
 */
#include "ITurbSimReader.h"

class BladedBinaryReader final : public ITurbSimReader
{
public:
    /**
     * @brief Parse a Bladed/AeroDyn full-field binary (.wnd) file.
     *
     * @param filename  Path to the .wnd file.
     * @return          Fully populated TurbSimFileData.
     * @throws std::runtime_error on I/O error or malformed file.
     */
    TurbSimFileData Read(std::string const &filename) const override;
};
