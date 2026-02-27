#include <chrono>
#include <iostream>
#include <memory>
#include <numbers>
#include <vector>

#include "ConfigurationSchema.h"
#include "FileReader.h"
#include "ConfigurationParser.h"
#include "ExporterFactory.h"
#include "DXFBlade3D.h"
#include "TurbineGeometry.h"

// ── Simulation layer ──────────────────────────────────────────────────────────
#include "ConfigurationAdapter.h"
#include "FlowCalculatorFactory.h"
#include "NingSolverFactory.h"
#include "VariableSpeedController.h"
#include "StandardPitchSchedule.h"
#include "EfficiencyModels.h"
#include "OperationSolver.h"
#include "AEPCalculator.h"
#include "BEMPostprocessor.h"

// ── Output layer ──────────────────────────────────────────────────────────────
#include "TecplotFormatter.h"
#include "ISimulationResultsExporter.h"
#include "TecplotSimulationExporter.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static void waitForKeyPress()
{
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

/// Print a single timing line with consistent column width.
static void printTiming(int step, std::string_view label,
                        std::chrono::steady_clock::time_point t0,
                        std::chrono::steady_clock::time_point t1,
                        std::string_view extra = "")
{
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double s = ms / 1000.0;

    std::cout << "[" << step << "] " << label;
    for (int i = static_cast<int>(label.size()); i < 26; ++i)
        std::cout << ' ';
    std::cout << ": " << ms << " ms (" << s << " s)";
    if (!extra.empty())
        std::cout << "  (" << extra << ")";
    std::cout << '\n';
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Wind turbine performance simulation using SOLID principles.
 */
int main(int /*argc*/, char **argv)
{
    auto t_total_start = std::chrono::steady_clock::now();

    try
    {
        // ── 1. Schema ─────────────────────────────────────────────────────────
        auto t0 = std::chrono::steady_clock::now();

        ConfigurationSchema schema;

        // Project data
        schema.addString("project_name", true, "Name of project");
        schema.addString("project_id", true, "Project number for identification");
        schema.addString("project_revision", true, "Project revision");
        schema.addString("project_date", true, "Date of simulation run");
        schema.addString("project_engineer", true, "Engineer responsible for simulation");

        // Data tables
        schema.addDataFile("airfoil_geometry_files_file", true, "Path to airfoil geometry data file list");
        schema.addDataFile("airfoil_performance_files_file", true, "Path to airfoil performance data file list");
        schema.addDataFile("blade_geometry_file", true, "Path to blade geometry data");

        // Turbine data
        schema.addBool("turbine_is_horizontal", true, "Flag for horizontal or vertical turbine");
        schema.addInt("number_of_blades", true, "Blade count");
        schema.addDouble("hub_radius", true, "Turbine hub radius [m]");
        schema.addDouble("cone", true, "Cone angle [deg]");
        schema.addDouble("yaw", true, "Yaw angle [deg]");
        schema.addDouble("tilt", true, "Tilt angle [deg]");
        schema.addDouble("tower_dist", true, "Tower distance [m]");
        schema.addDouble("hub_height", true, "Hub height [m]");

        // Operation data
        schema.addDouble("rated_rotorspeed", true, "Rotor speed at rated conditions [rpm]");
        schema.addDouble("min_rotorspeed", true, "Cut-in min rotor speed [rpm]");
        schema.addDouble("rated_electrical_power", true, "Rated electrical power [W]");

        // Simulation data
        schema.addBool("simulation_is_time_based", true, "Flag for static or time-based simulation");
        schema.addRange("wind_speed_range",
                        "windspeed_start", "windspeed_end", "windspeed_step",
                        true, "Wind speed range for static simulation [m/s]");
        schema.addRange("tip_speed_ratio_range",
                        "tsr_start", "tsr_end", "tsr_step",
                        true, "Tip speed ratio range [-]");
        schema.addRange("pitch_angle_range",
                        "pitch_start", "pitch_emd", "pitch_step",
                        true, "Pitch angle range [deg]");

        // Solver / controller keys
        schema.addDouble("n_nenn", true, "Maximum rotor speed [rpm]");
        schema.addDouble("lambda_opt", true, "Optimal tip-speed ratio [-]");
        schema.addDouble("max_dp_dn", true, "Max dP/dn gradient [W/rpm]");
        schema.addString("power_mode", true, "Speed control mode: L0 or POWER");
        schema.addDouble("kinematic_viscosity", true, "Air kinematic viscosity [m^2/s]");
        schema.addDouble("speed_of_sound", true, "Speed of sound [m/s]");
        schema.addDouble("air_density", true, "Air density [kg/m^3]");
        schema.addDouble("convergence_tol", true, "BEM convergence tolerance");
        schema.addDouble("wake_transition", true, "Empirical wake transition point");
        schema.addDouble("tip_extra_dist", true, "Tip singularity extra distance [m]");

        // AEP keys
        schema.addDouble("aep_k_factor", true, "Weibull k factor for AEP");
        schema.addDouble("aep_money_kwh", true, "Energy price [EUR/kWh]");
        schema.addRange("aep_vmean_range",
                        "vmean_start", "vmean_end", "vmean_step",
                        true, "Mean wind speed range for AEP [m/s]");

        auto t1 = std::chrono::steady_clock::now();
        printTiming(1, "Schema built", t0, t1);

        // ── 2. Parse ──────────────────────────────────────────────────────────
        auto fileReader = std::make_unique<FileReader>(argv[1]);
        ConfigurationParser parser(std::move(schema), std::move(fileReader));
        Configuration config = parser.parse();

        auto t2 = std::chrono::steady_clock::now();
        printTiming(2, "Config parsed", t1, t2);

        // ── 3. Build TurbineGeometry ──────────────────────────────────────────
        [[maybe_unused]] const BladeGeometryData *bladeGeometry = config.getBladeGeometry();
        [[maybe_unused]] const AirfoilPerformanceFileListData *airfoilPerformanceFileList = config.getAirfoilPerformanceFileList();
        [[maybe_unused]] const AirfoilGeometryFileListData *airfoilGeometryFileList = config.getAirfoilGeometryFileList();

        std::unique_ptr<BladeInterpolator> bladeInterpolator = config.createBladeInterpolator();

        [[maybe_unused]] double ratedRotorSpeed = config.getDouble("rated_rotorspeed");
        [[maybe_unused]] bool isHorizontal = config.getBool("turbine_is_horizontal");
        [[maybe_unused]] bool isTimeBased = config.getBool("simulation_is_time_based");

        std::cout << "Configuration loaded successfully.\n";

        auto turbine = std::make_unique<TurbineGeometry>(std::move(bladeInterpolator));
        turbine->setTurbineConfiguration(
            config.getDouble("hub_radius"),
            config.getDouble("cone"),
            config.getDouble("yaw"),
            config.getDouble("tilt"),
            config.getDouble("tower_dist"),
            config.getDouble("hub_height"),
            config.getInt("number_of_blades"));
        turbine->PreComputeRotationMatrices();
        turbine->set_number_of_blades(config.getInt("number_of_blades"));

        auto t3 = std::chrono::steady_clock::now();
        printTiming(3, "TurbineGeometry built", t2, t3,
                    std::to_string(turbine->num_sections()) + " sections");

        // ── 4. Wrap Configuration as ISimulationConfig ────────────────────────
        ConfigurationAdapter sim_config(config);

        auto t4 = std::chrono::steady_clock::now();
        printTiming(4, "SimConfig wrapped", t3, t4);

        // ── 5. Build controller ───────────────────────────────────────────────
        auto ctrl_params = TurbineControllerParams::FromConfig(turbine.get(), &sim_config);

        auto pitch_sched = std::make_unique<StandardPitchSchedule>(
            0.0,                       // base pitch [rad]
            std::vector<double>{0.0},  // P_min breakpoints [W]
            std::vector<double>{0.0}); // delta-pitch values [rad]

        auto eta_model = std::make_unique<ConstantEfficiency>(0.94);
        auto controller = std::make_unique<VariableSpeedController>(
            std::move(ctrl_params),
            pitch_sched.get(),
            eta_model.get());

        auto t5 = std::chrono::steady_clock::now();
        printTiming(5, "Controller built", t4, t5);

        // ── 6. Build OperationSolver ──────────────────────────────────────────
        auto op_params = OperationSolverParams::FromConfig(turbine.get(), &sim_config);

        // Wind speed vector from config range
        std::vector<double> vinf_vec;
        for (double v = sim_config.wind_speed_start();
             v <= sim_config.wind_speed_end() + sim_config.wind_speed_step() * 1e-9;
             v += sim_config.wind_speed_step())
            vinf_vec.push_back(v);

        FlowCalculatorFactory fc_factory;
        NingSolverFactory solver_factory;
        const double psi = 0.0; // steady-state: single azimuth

        // Collect postprocessor results per wind speed for rotor disc export.
        std::vector<BEMPostprocessResult> pp_vec;
        pp_vec.reserve(vinf_vec.size());

        // BEM callback: FlowCalculator + NingSolver + BEMPostprocessor per point.
        BEMCallback bem_callback = [&](double vinf,
                                       double lambda,
                                       double pitch_deg)
            -> std::pair<double, double>
        {
            double rot_rate = lambda * vinf / turbine->RotorRadius();
            double pitch_rad = pitch_deg * std::numbers::pi / 180.0;

            auto fc = fc_factory.Build(turbine.get(), rot_rate, vinf, psi, FlowModifiers{});
            auto solver = solver_factory.Build(turbine.get(), &sim_config, fc.get(), pitch_rad, psi);

            if (!solver->Solve())
                return {0.0, 0.0};

            BEMPostprocessor postproc(
                turbine.get(),
                &sim_config,
                fc.get(),
                static_cast<double>(turbine->num_blades()));

            postproc.Process(*solver);

            if (!postproc.Success())
                return {0.0, 0.0};

            BEMPostprocessResult const &pp = postproc.Result();

            // Store result for rotor disc export (one entry per wind speed).
            pp_vec.push_back(pp);

            return {pp.cp, pp.ct};
        };

        OperationSolver op_solver(op_params, controller.get(), bem_callback);

        auto t6 = std::chrono::steady_clock::now();
        printTiming(6, "OperationSolver built", t5, t6,
                    std::to_string(vinf_vec.size()) + " wind speed points");

        // ── 7. Run power curve ────────────────────────────────────────────────
        auto power_curve = op_solver.Run(0.0, vinf_vec);

        auto t7 = std::chrono::steady_clock::now();
        printTiming(7, "Power curve solved", t6, t7,
                    std::to_string(vinf_vec.size()) + " wind speed points");

        // ── 8. AEP ────────────────────────────────────────────────────────────
        std::vector<double> pel_vec;
        pel_vec.reserve(power_curve.size());
        for (auto const &pt : power_curve)
            pel_vec.push_back(pt.p_el);

        AEPCalculator aep_calc(
            vinf_vec, pel_vec,
            sim_config.wind_speed_bin_width(),
            sim_config.weibull_k(),
            sim_config.energy_price_per_kwh());

        auto vmean_vec = sim_config.mean_wind_speeds();
        auto aep_results = aep_calc.ComputeRange(vmean_vec);

        auto t8 = std::chrono::steady_clock::now();
        printTiming(8, "AEP computed", t7, t8,
                    std::to_string(vmean_vec.size()) + " mean wind speed(s)");

        // ── 9. Export results ─────────────────────────────────────────────────
        //
        // One shared TecplotFormatter, injected into the exporter.
        // Swap formatter for a different output format without touching main.
        //
        auto formatter = std::make_shared<TecplotFormatter>();
        std::unique_ptr<ISimulationResultsExporter> exporter =
            std::make_unique<TecplotSimulationExporter>(formatter);

        // turbine_performance.dat — power curve, one row per wind speed
        if (exporter->ExportPowerCurve(power_curve, "output/turbine_performance.dat"))
            std::cout << "  -> output/turbine_performance.dat written\n";
        else
            std::cerr << "  -> turbine_performance.dat FAILED\n";

        // blade_data.dat — section data at the rated wind speed (last point)
        if (!pp_vec.empty() && !power_curve.empty())
        {
            // Pick the operating point closest to rated power for blade export.
            std::size_t rated_idx = 0;
            double p_max = 0.0;
            for (std::size_t i = 0; i < power_curve.size(); ++i)
            {
                if (power_curve[i].p_el > p_max)
                {
                    p_max = power_curve[i].p_el;
                    rated_idx = i;
                }
            }
            // Guard against pp_vec being shorter than power_curve (convergence
            // failures may have skipped a postprocessor push).
            std::size_t blade_idx = std::min(rated_idx, pp_vec.size() - 1);

            if (exporter->ExportBladeData(pp_vec[blade_idx], turbine.get(),
                                          vinf_vec[blade_idx], "output/blade_data.dat"))
                std::cout << "  -> output/blade_data.dat written"
                          << "  (v_inf = " << vinf_vec[blade_idx] << " m/s)\n";
            else
                std::cerr << "  -> blade_data.dat FAILED\n";
        }

        // rotor_disc_data.dat — all wind speeds, one zone per wind speed
        if (!pp_vec.empty())
        {
            if (exporter->ExportRotorDiscData(pp_vec, turbine.get(),
                                              vinf_vec, "output/rotor_disc_data.dat"))
                std::cout << "  -> output/rotor_disc_data.dat written"
                          << "  (" << pp_vec.size() << " zones)\n";
            else
                std::cerr << "  -> rotor_disc_data.dat FAILED\n";
        }

        auto t9 = std::chrono::steady_clock::now();
        printTiming(9, "Results exported", t8, t9,
                    "3 files");

        // ── Summary ───────────────────────────────────────────────────────────
        double total_ms = std::chrono::duration<double, std::milli>(
                              t9 - t_total_start)
                              .count();

        std::cout << "\n"
                  << "Total simulation time      : "
                  << total_ms << " ms (" << total_ms / 1000.0 << " s)\n"
                  << "Simulation complete.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        waitForKeyPress();
        return 1;
    }

    waitForKeyPress();
    return 0;
}
