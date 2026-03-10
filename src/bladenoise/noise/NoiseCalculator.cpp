#include "bladenoise/noise/NoiseCalculator.h"
#include "bladenoise/noise/TBLTENoiseSource.h"
#include "bladenoise/noise/TurbulentInflowNoiseSource.h"
#include "bladenoise/noise/BluntnessNoiseSource.h"
#include "bladenoise/airfoil/BPMBoundaryLayerCalculator.h"
#include "bladenoise/math/SpecialFunctions.h"
#include "bladenoise/math/Spline.h"
#include "bladenoise/core/Constants.h"
#include "bladenoise/core/Types.h"
#include "bladenoise/potential/PotentialFlowSolver.h"
#include <iostream>
#include <cmath>

namespace bladenoise
{
    namespace noise
    {

        using namespace constants;
        using math::MathUtils;

        NoiseCalculator::NoiseCalculator()
        {
            // Initialize frequency bands
            frequencies_.assign(THIRD_OCTAVE_BANDS.begin(), THIRD_OCTAVE_BANDS.end());
        }

        NoiseCalculator::~NoiseCalculator() = default;

        void NoiseCalculator::combine_noise_sources(
            const std::vector<NoiseResult> &sources,
            NoiseResult &total)
        {
            const size_t num_freq = frequencies_.size();
            total.spl.resize(num_freq, 0.0);

            for (size_t i = 0; i < num_freq; ++i)
            {
                Real pressure_sum = 0.0;
                for (const auto &source : sources)
                {
                    if (i < source.spl.size() && source.spl[i] > -100.0)
                    {
                        pressure_sum += MathUtils::from_dB(source.spl[i]);
                    }
                }
                if (pressure_sum > 0.0)
                {
                    total.spl[i] = MathUtils::to_dB(pressure_sum);
                }
                else
                {
                    total.spl[i] = -100.0;
                }
            }

            total.overall_spl = MathUtils::compute_OASPL(total.spl);
        }

        bool NoiseCalculator::calculate_streamlines_full(
            const ProjectConfig &config,
            const io::AirfoilData &airfoil)
        {
            // Create potential flow solver
            potential_solver_ = std::make_unique<potential::PotentialFlowSolver>(
                // static_cast<int>(airfoil.num_points) - 1);
                200);

            //TODO: debug output: std::cout << "Setting up potential flow geometry...\n";
            if (!potential_solver_->setup_geometry(airfoil, config))
            {
                error_message_ = "Failed to setup potential flow geometry: " +
                                 potential_solver_->get_error();
                return false;
            }

            //TODO: debug output: std::cout << "Solving potential flow...\n";
            if (!potential_solver_->solve())
            {
                error_message_ = "Failed to solve potential flow: " +
                                 potential_solver_->get_error();
                return false;
            }

            //TODO: debug output: std::cout << "Computing streamlines...\n";
            if (!potential_solver_->calculate_streamlines(config.num_streamlines,
                                                          config.streamline_spacing,
                                                          streamlines_))
            {
                error_message_ = "Failed to compute streamlines: " +
                                 potential_solver_->get_error();
                return false;
            }

            return true;
        }

        bool NoiseCalculator::calculate(
            const ProjectConfig &config,
            const io::AirfoilData &airfoil,
            CombinedNoiseResults &results)
        {
            // Clear previous streamline data
            streamlines_.clear();

            // Initialize all results
            const size_t num_freq = frequencies_.size();
            results.tbl_pressure_side = NoiseResult(num_freq);
            results.tbl_suction_side = NoiseResult(num_freq);
            results.separation = NoiseResult(num_freq);
            results.laminar_vortex = NoiseResult(num_freq);
            results.bluntness = NoiseResult(num_freq);
            results.turbulent_inflow = NoiseResult(num_freq);
            results.total = NoiseResult(num_freq);

            // Create boundary layer calculator
            auto bl_calculator = airfoil::create_boundary_layer_calculator(config.bl_method);

            BoundaryLayerState upper_bl, lower_bl;
            if (!bl_calculator->calculate(airfoil, config, upper_bl, lower_bl))
            {
                error_message_ = "Boundary layer calculation failed: " + bl_calculator->get_error();
                return false;
            }

            //TODO: debug output: std::cout << "Boundary layer calculation complete:\n";
            //TODO: debug output: std::cout << "  Upper delta*: " << upper_bl.displacement_thickness * 1000.0 << " mm\n";
            //TODO: debug output: std::cout << "  Lower delta*: " << lower_bl.displacement_thickness * 1000.0 << " mm\n";
            //TODO: debug output: std::cout << "  Upper H:      " << upper_bl.shape_factor << "\n";
            //TODO: debug output: std::cout << "  Lower H:      " << lower_bl.shape_factor << "\n";

            std::vector<NoiseResult> all_sources;

            // TBL-TE noise (includes LBL-VS when compute_laminar is set)
            if (config.tbl_method != TBLNoiseMethod::NONE)
            {
                //TODO: debug output: std::cout << "Computing TBL-TE noise...\n";
                TBLTENoiseSource tbl_source;
                NoiseResult tbl_result(num_freq);

                if (tbl_source.calculate(config, upper_bl, lower_bl, frequencies_, tbl_result))
                {
                    results.tbl_pressure_side = tbl_source.pressure_side_result;
                    results.tbl_suction_side = tbl_source.suction_side_result;
                    results.separation = tbl_source.separation_result;
                    results.laminar_vortex = tbl_source.laminar_result;
                    all_sources.push_back(tbl_result);

                    //TODO: debug output: std::cout << "  TBL-TE OASPL: " << tbl_result.overall_spl << " dB\n";
                    if (config.compute_laminar)
                    {
                        //TODO: debug output: std::cout << "  LBL-VS OASPL: " << tbl_source.laminar_result.overall_spl << " dB\n";
                    }
                }
            }

            // Turbulent inflow noise
            if (config.ti_method != TINoiseMethod::NONE)
            {
                //TODO: debug output: std::cout << "Computing turbulent inflow noise...\n";

                // Calculate streamlines if using full Guidati method
                if (config.ti_method == TINoiseMethod::GUIDATI)
                {
                    //TODO: debug output: std::cout << "Using full potential flow solver for Guidati method...\n";
                    if (!calculate_streamlines_full(config, airfoil))
                    {
                        std::cerr << "Warning: Streamline calculation failed, using simplified method\n";
                    }
                }

                TurbulentInflowNoiseSource ti_source(config.ti_method);

                if (ti_source.calculate(config, upper_bl, lower_bl, frequencies_,
                                        results.turbulent_inflow))
                {
                    all_sources.push_back(results.turbulent_inflow);
                    //TODO: debug output: std::cout << "  TI OASPL: " << results.turbulent_inflow.overall_spl << " dB\n";
                }
            }

            // Bluntness noise
            if (config.compute_bluntness && config.trailing_edge_thickness > 0.0)
            {
                //TODO: debug output: std::cout << "Computing bluntness noise...\n";
                BluntnessNoiseSource blunt_source;

                if (blunt_source.calculate(config, upper_bl, lower_bl, frequencies_,
                                           results.bluntness))
                {
                    all_sources.push_back(results.bluntness);
                    //TODO: debug output: std::cout << "  Bluntness OASPL: " << results.bluntness.overall_spl << " dB\n";
                }
            }

            // Combine all noise sources
            combine_noise_sources(all_sources, results.total);

            // TODO: debug output: std::cout << "Noise calculation complete for airfoil: " << airfoil.name << "\n";
            // TODO: debug output: std::cout << "  Total OASPL: " << results.total.overall_spl << " dB\n";
            return true;
        }

    } // namespace noise
} // namespace bladenoise
