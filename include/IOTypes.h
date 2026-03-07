#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"
#include <string>
#include <vector>
#include <memory>

namespace bladenoise
{
    namespace io
    {

        // Airfoil coordinate data
        struct AirfoilData
        {
            std::string name;
            RealVector x;
            RealVector y;
            size_t num_points = 0;
            bool is_closed = false;

            void clear()
            {
                name.clear();
                x.clear();
                y.clear();
                num_points = 0;
                is_closed = false;
            }
        };

        // Streamline data from potential flow solver
        struct StreamlineData
        {
            int num_streamlines = 0;
            int points_per_streamline = 0;
            Real streamline_spacing = 0.0;
            Real time_step = 0.0;
            Real stagnation_x = 0.0;
            Real stagnation_y = 0.0;

            RealVector time;
            std::vector<RealVector> x;
            std::vector<RealVector> y;
            std::vector<RealVector> potential;

            void resize(int npath, int nstr)
            {
                num_streamlines = npath;
                points_per_streamline = nstr;
                time.resize(nstr, 0.0);
                x.resize(npath, RealVector(nstr, 0.0));
                y.resize(npath, RealVector(nstr, 0.0));
                potential.resize(npath, RealVector(nstr, 0.0));
            }

            void clear()
            {
                num_streamlines = 0;
                points_per_streamline = 0;
                time.clear();
                x.clear();
                y.clear();
                potential.clear();
            }

            bool empty() const
            {
                return num_streamlines == 0 || points_per_streamline == 0;
            }
        };

        // Interface for airfoil file readers
        class IAirfoilReader
        {
        public:
            virtual ~IAirfoilReader() = default;
            virtual bool read(const std::string &filename, AirfoilData &data) = 0;
            virtual std::string get_error() const = 0;
        };

        // Interface for config file readers
        class IConfigReader
        {
        public:
            virtual ~IConfigReader() = default;
            virtual bool read(const std::string &filename, ProjectConfig &config) = 0;
            virtual std::string get_error() const = 0;
        };

        // Interface for results writers
        class IResultsWriter
        {
        public:
            virtual ~IResultsWriter() = default;
            virtual bool write(const std::string &filename,
                               const RealVector &frequencies,
                               const CombinedNoiseResults &results) = 0;
            virtual std::string get_error() const = 0;
        };

        // Factory functions
        std::unique_ptr<IAirfoilReader> create_airfoil_reader();
        std::unique_ptr<IConfigReader> create_config_reader(const std::string &filename);
        std::unique_ptr<IResultsWriter> create_results_writer();

    } // namespace io
} // namespace bladenoise
