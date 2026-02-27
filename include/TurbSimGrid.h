#pragma once
/**
 * @file TurbSimGrid.h
 * @brief Geometry of a TurbSim wind-field grid.
 *
 * Single Responsibility: knows only the spatial layout of the grid — the
 * y-axis and z-axis sample points.  No velocity data, no file I/O.
 *
 * This is a plain value type; it may be copied freely.
 */
#include <cstddef>
#include <stdexcept>
#include <vector>

class TurbSimGrid
{
public:
    // ── Default construction (empty grid) ────────────────────────────────────
    TurbSimGrid() = default;

    /**
     * @brief Construct a TurbSim grid from the parameters found in a .wnd file.
     *
     * @param centre_height  Hub height above ground [m].
     * @param spacing_y      Lateral grid spacing [m].
     * @param spacing_z      Vertical grid spacing [m].
     * @param num_y          Number of grid points in the y direction.
     * @param num_z          Number of grid points in the z direction.
     */
    TurbSimGrid(double centre_height,
                double spacing_y,
                double spacing_z,
                unsigned num_y,
                unsigned num_z)
        : num_y_(num_y), num_z_(num_z)
    {
        if (num_y == 0 || num_z == 0)
            throw std::invalid_argument("TurbSimGrid: grid dimensions must be > 0");
        if (spacing_y <= 0.0 || spacing_z <= 0.0)
            throw std::invalid_argument("TurbSimGrid: grid spacings must be > 0");

        BuildGridPoints(centre_height, spacing_y, spacing_z);
    }

    // ── Dimensions ───────────────────────────────────────────────────────────
    unsigned num_y() const { return num_y_; }
    unsigned num_z() const { return num_z_; }
    std::size_t total_points() const { return static_cast<std::size_t>(num_y_) * num_z_; }

    // ── Axis vectors (one value per grid line) ────────────────────────────────
    /**
     * @brief Unique y-axis grid positions (num_y values).
     */
    std::vector<double> axis_y() const
    {
        std::vector<double> y(num_y_);
        for (unsigned i = 0; i < num_y_; ++i)
            y[i] = grid_points_y_[i]; // first row holds unique y values
        return y;
    }

    /**
     * @brief Unique z-axis grid positions (num_z values).
     */
    std::vector<double> axis_z() const
    {
        std::vector<double> z(num_z_);
        for (unsigned j = 0; j < num_z_; ++j)
            z[j] = grid_points_z_[j * num_y_]; // one per row
        return z;
    }

    // ── Full flat arrays (num_y × num_z values, row-major) ────────────────────
    std::vector<double> const &grid_points_y() const { return grid_points_y_; }
    std::vector<double> const &grid_points_z() const { return grid_points_z_; }

private:
    unsigned num_y_{0};
    unsigned num_z_{0};
    std::vector<double> grid_points_y_;
    std::vector<double> grid_points_z_;

    void BuildGridPoints(double centre_height,
                         double spacing_y,
                         double spacing_z)
    {
        double width = (num_y_ - 1) * spacing_y;
        double height = (num_z_ - 1) * spacing_z;

        grid_points_y_.reserve(total_points());
        grid_points_z_.reserve(total_points());

        for (unsigned j = 0; j < num_z_; ++j)
        {
            for (unsigned i = 0; i < num_y_; ++i)
            {
                grid_points_y_.push_back(i * spacing_y - width / 2.0);
                grid_points_z_.push_back(centre_height - height / 2.0 + j * spacing_z);
            }
        }
    }
};
