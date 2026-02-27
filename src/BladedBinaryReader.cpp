/**
 * @file BladedBinaryReader.cpp
 * @brief Implementation of BladedBinaryReader.
 *
 * All binary file parsing for the Bladed/AeroDyn .wnd format lives here.
 * Nothing else needs to change when the format changes.
 *
 * Reference: TurbSim User's Guide, Appendix E —
 *   "Full-Field Bladed Binary Format (.wnd)"
 */
#include "BladedBinaryReader.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Read
// ─────────────────────────────────────────────────────────────────────────────
TurbSimFileData BladedBinaryReader::Read(std::string const &filename) const
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
        throw std::runtime_error("BladedBinaryReader: cannot open file: " + filename);

    // ── Hub height and turbulence intensities (offset 16) ────────────────────
    in.seekg(16);

    auto read_float = [&]()
    {
        float v{};
        in.read(reinterpret_cast<char *>(&v), sizeof(v));
        if (!in)
            throw std::runtime_error("BladedBinaryReader: unexpected end of file");
        return static_cast<double>(v);
    };

    double centre_height = read_float(); // hub height [m]
    double ti_u = read_float();          // turbulence intensity u [%]
    double ti_v = read_float();          // turbulence intensity v [%]
    double ti_w = read_float();          // turbulence intensity w [%]

    // ── Grid spacing (offsets follow immediately) ─────────────────────────────
    double spacing_z = read_float();        // vertical grid spacing [m]
    double spacing_y = read_float();        // lateral grid spacing  [m]
    double longitudinal_res = read_float(); // longitudinal Δx [m]

    // ── Number of timesteps (offset 44) ──────────────────────────────────────
    in.seekg(44);
    {
        int32_t r{};
        in.read(reinterpret_cast<char *>(&r), sizeof(r));
        if (!in)
            throw std::runtime_error("BladedBinaryReader: cannot read nt");
        // TurbSim stores half the timesteps; actual count is nt * 2.
        std::size_t nt = static_cast<std::size_t>(r) * 2;

        // ── Hub velocity ──────────────────────────────────────────────────────
        double hub_velocity = read_float();

        // ── Grid dimensions (offset 72) ───────────────────────────────────────
        in.seekg(72);
        int32_t num_z_raw{}, num_y_raw{};
        in.read(reinterpret_cast<char *>(&num_z_raw), sizeof(num_z_raw));
        in.read(reinterpret_cast<char *>(&num_y_raw), sizeof(num_y_raw));
        if (!in)
            throw std::runtime_error("BladedBinaryReader: cannot read grid dims");

        auto num_z = static_cast<unsigned>(num_z_raw);
        auto num_y = static_cast<unsigned>(num_y_raw);

        // ── Construct grid and timing objects ─────────────────────────────────
        TurbSimGrid grid(centre_height, spacing_y, spacing_z, num_y, num_z);

        double grid_width_y = static_cast<double>(num_y - 1) * spacing_y;
        TurbSimTimingInfo timing(nt, hub_velocity, longitudinal_res, grid_width_y);

        // ── Read velocity data (offset 104) ───────────────────────────────────
        in.seekg(104);
        TurbSimVelocityData velocity;

        for (std::size_t i = 0; i < nt; ++i)
        {
            VelocityTimestep vt;
            vt.reserve(static_cast<std::size_t>(num_z) * num_y);

            for (unsigned z = 0; z < num_z; ++z)
            {
                for (unsigned y = 0; y < num_y; ++y)
                {
                    auto decode = [&](double /*ti_pct*/)
                    {
                        int16_t buf{};
                        in.read(reinterpret_cast<char *>(&buf), sizeof(buf));
                        return buf;
                    };

                    int16_t raw_u = decode(ti_u);
                    int16_t raw_v = decode(ti_v);
                    int16_t raw_w = decode(ti_w);

                    // Bladed encoding:  v = hub_vel * ((TI/100) / 1000 * raw + offset)
                    double u = hub_velocity * ((ti_u / 100.0) / 1000.0 * raw_u + 1.0);
                    double v = hub_velocity * ((ti_v / 100.0) / 1000.0 * raw_v);
                    double w = hub_velocity * ((ti_w / 100.0) / 1000.0 * raw_w);

                    vt.emplace_back(u, v, w);
                }
            }
            velocity.AddTimestep(std::move(vt));
        }

        if (!in)
            throw std::runtime_error("BladedBinaryReader: file truncated during velocity read");

        return TurbSimFileData(
            std::move(grid),
            std::move(velocity),
            std::move(timing),
            hub_velocity);
    }
}
