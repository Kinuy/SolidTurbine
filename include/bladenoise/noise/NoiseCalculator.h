#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"
#include "bladenoise/io/IOTypes.h"
#include <memory>
#include <string>
#include <vector>

namespace bladenoise
{

    // Forward declarations
    namespace potential
    {
        class PotentialFlowSolver;
    }

    namespace noise
    {

        class NoiseCalculator
        {
        public:
            NoiseCalculator();
            ~NoiseCalculator();

            bool calculate(
                const ProjectConfig &config,
                const io::AirfoilData &airfoil,
                CombinedNoiseResults &results);

            const RealVector &frequencies() const { return frequencies_; }
            std::string get_error() const { return error_message_; }

            bool has_streamlines() const { return !streamlines_.empty(); }
            const io::StreamlineData &streamlines() const { return streamlines_; }

        private:
            void combine_noise_sources(const std::vector<NoiseResult> &sources,
                                       NoiseResult &total);

            bool calculate_streamlines_full(const ProjectConfig &config,
                                            const io::AirfoilData &airfoil);

            RealVector frequencies_;
            io::StreamlineData streamlines_;
            std::unique_ptr<potential::PotentialFlowSolver> potential_solver_;
            std::string error_message_;
        };

    } // namespace noise
} // namespace bladenoise
