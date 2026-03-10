#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <cmath>
#include <iomanip>
#include <numbers>
#include <optional>
#include <vector>

#include "ConfigurationSchema.h"
#include "FileReader.h"
#include "ConfigurationParser.h"
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
#include "IBlade3DExporter.h"
#include "TecplotBlade3DExporter.h"
#include "DXFBlade3DExporter.h"
#include "ISimulationResultsExporter.h"
#include "TecplotSimulationExporter.h"
#include "RotormapSolver.h"
#include "SectionNoiseCalculator.h"
#include "BEMSectionNoiseAdapter.h"
#include "BladeNoiseConfigBuilder.h"
#include "INoiseResultsExporter.h"
#include "TecplotNoiseExporter.h"


// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static void waitForKeyPress()
{
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

static void printTiming(int step, std::string_view label,
                        std::chrono::steady_clock::time_point t0,
                        std::chrono::steady_clock::time_point t1,
                        std::string_view extra = "")
{
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double s  = ms / 1000.0;

    std::cout << "[" << step << "] " << label;
    for (int i = static_cast<int>(label.size()); i < 26; ++i)
        std::cout << ' ';
    std::cout << ": ";
    if (s >= 60.0)
    {
        double min     = std::floor(s / 60.0);
        double sec_rem = s - min * 60.0;
        std::cout << std::fixed << std::setprecision(0) << min << " min "
                  << std::setprecision(1) << sec_rem << " s";
    }
    else
    {
        std::cout << std::defaultfloat << ms << " ms (" << s << " s)";
    }
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
        schema.addDataFile("turbine_controller_file", true, "Path to turbine controller data");

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
        schema.addDouble("temperature", true, "Temperature [K]");
        schema.addDouble("convergence_tol", true, "BEM convergence tolerance");
        schema.addDouble("wake_transition", true, "Empirical wake transition point");
        schema.addDouble("tip_extra_dist", true, "Tip singularity extra distance [m]");
        schema.addDouble("rotor_azimuth_psi_increment", true,
                         "Psi azimuth step [deg]: 0=scalar at psi=0, >0 builds vector [0:step:360)");

        // AEP keys
        schema.addDouble("aep_k_factor", true, "Weibull k factor for AEP");
        schema.addDouble("aep_money_kwh", true, "Energy price [EUR/kWh]");
        schema.addRange("aep_vmean_range",
                        "vmean_start", "vmean_end", "vmean_step",
                        true, "Mean wind speed range for AEP [m/s]");

        // Rotormap keys
        schema.addDouble("rotormap_v_tip",       true, "Constant tip speed for Rotormap [m/s]");
        schema.addRange ("rotormap_lambda_range",
                         "rotormap_lambda_start", "rotormap_lambda_end", "rotormap_lambda_step",
                         true, "TSR range for Rotormap sweep [-]");
        schema.addRange ("rotormap_pitch_range",
                         "rotormap_pitch_start", "rotormap_pitch_end", "rotormap_pitch_step",
                         true, "Pitch range for Rotormap sweep [deg]");

        // ── Noise calculation parameters ──────────────────────────────────────
        schema.addInt("noise_bl_tripping", true,
                      "BL trip: 0=no trip (BPM), 1=heavy trip (BPM only), 2=light trip");
        schema.addInt("noise_bl_properties_calc_method", true,
                      "BL properties method: 1=BPM, 2=Xfoil");
        schema.addInt("noise_tbl_noise_calc_method", true,
                      "TBL noise method: 0=none, 1=BPM, 2=TNO");
        schema.addInt("noise_ti_noise_calc_method", true,
                      "TI noise method: 0=none, 1=Amiet (flat plate), 2=Guidati, 3=Simplified");
        schema.addBool("noise_calc_blunt_te_noise", true,
                       "Compute bluntness TE noise: 0=No, 1=Yes");
        schema.addBool("noise_calc_lam_bl_noise", true,
                       "Compute LBL noise: 0=No, 1=Yes");

        auto t1 = std::chrono::steady_clock::now();
        printTiming(1, "Schema built", t0, t1);

        // ── 2. Parse ──────────────────────────────────────────────────────────
        auto fileReader = std::make_unique<FileReader>(argv[1]);
        ConfigurationParser parser(std::move(schema), std::move(fileReader));
        Configuration config = parser.parse();

        [[maybe_unused]] const BladeGeometryData *bladeGeometry = config.getBladeGeometry();
        [[maybe_unused]] const AirfoilPerformanceFileListData *airfoilPerformanceFileList = config.getAirfoilPerformanceFileList();
        [[maybe_unused]] const AirfoilGeometryFileListData *airfoilGeometryFileList = config.getAirfoilGeometryFileList();

        [[maybe_unused]] double ratedRotorSpeed = config.getDouble("rated_rotorspeed");
        [[maybe_unused]] bool isHorizontal = config.getBool("turbine_is_horizontal");
        [[maybe_unused]] bool isTimeBased = config.getBool("simulation_is_time_based");

        // ── Noise parameters ──────────────────────────────────────────────────
        int  noise_bl_tripping              = config.getInt("noise_bl_tripping");
        int  noise_bl_properties_calc_method = config.getInt("noise_bl_properties_calc_method");
        int  noise_tbl_noise_calc_method     = config.getInt("noise_tbl_noise_calc_method");
        int  noise_ti_noise_calc_method      = config.getInt("noise_ti_noise_calc_method");
        bool noise_calc_blunt_te_noise       = config.getBool("noise_calc_blunt_te_noise");
        bool noise_calc_lam_bl_noise         = config.getBool("noise_calc_lam_bl_noise");

        std::cout << "Configuration loaded successfully.\n";

        auto t2 = std::chrono::steady_clock::now();
        printTiming(2, "Config parsed", t1, t2);


            // ── 3. Blade geometry export (DXF + Tecplot 3D) ───────────────────────
        //
        // createBladeInterpolator() is called once per exporter so each can
        // take ownership independently.  config holds all raw data so each
        // call is cheap — no deep-copy machinery needed.
        //
        {
            namespace fs = std::filesystem;

            const std::string output_dir = "output";
            fs::create_directories(output_dir);

            // ── 3a. DXF blade geometry ───────────────────────────────────────
            // DXFBlade3DExporter implements IBlade3DExporter — path is passed
            // directly to Export(), no working-directory side-effects.
            {
                const std::string dxf_path = output_dir + "/blade3D_geometry.dxf";

                std::unique_ptr<IBlade3DExporter> dxfExporter =
                    std::make_unique<DXFBlade3DExporter>();

                auto dxfInterpolator = config.createBladeInterpolator();

                if (dxfExporter->Export(*dxfInterpolator, dxf_path))
                    std::cout << "  -> " << dxf_path << " written"
                              << "  (" << dxfInterpolator->getBladeSections().size()
                              << " sections)\n";
                else
                    std::cerr << "  -> " << dxf_path << " FAILED\n";
            }

            // ── 3b. Tecplot 3D blade geometry ─────────────────────────────────
            // One zone per airfoil section, variables X Y Z chord radius
            // rel_thickness.  Tecplot can animate over zones or display all
            // sections simultaneously as a 3D surface.
            {
                const std::string tec_path = output_dir + "/blade3D_geometry.dat";

                // Shared formatter — same TecplotFormatter used by all exporters.
                auto formatter = std::make_shared<TecplotFormatter>();
                std::unique_ptr<IBlade3DExporter> blade3DExporter =
                    std::make_unique<TecplotBlade3DExporter>(formatter);

                auto tecInterpolator = config.createBladeInterpolator();

                if (blade3DExporter->Export(*tecInterpolator, tec_path))
                    std::cout << "  -> " << tec_path << " written"
                              << "  (" << tecInterpolator->getBladeSections().size()
                              << " sections)\n";
                else
                    std::cerr << "  -> " << tec_path << " FAILED\n";
            }
        }

        auto t3 = std::chrono::steady_clock::now();
        printTiming(3, "Blade geometry exported", t2, t3, "3 files");

        // ── 4. Build TurbineGeometry ──────────────────────────────────────────
        //
        // Fresh interpolator — independent from the ones used in step 3.
        //
        auto bladeInterpolator = config.createBladeInterpolator();
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

        auto t4 = std::chrono::steady_clock::now();
        printTiming(4, "TurbineGeometry built", t3, t4,
                    std::to_string(turbine->num_sections()) + " sections");

        // ── 5. Wrap Configuration as ISimulationConfig ────────────────────────
        ConfigurationAdapter sim_config(config);

        auto t5 = std::chrono::steady_clock::now();
        printTiming(5, "SimConfig wrapped", t4, t5);

        // ── 6. Build controller from TurbineControlSettings ──────────────────
        //
        // ctrlSettings is loaded automatically by ConfigurationParser when it
        // processes "turbine_controller_file" from the schema — no manual
        // parsing needed in main.  Access is via config.getTurbineControlSettings().
        //
        const TurbineControlSettingsData* ctrlSettingsPtr =
            config.getTurbineControlSettings();
        if (!ctrlSettingsPtr)
            throw std::runtime_error(
                "Turbine controller settings not loaded — "
                "check 'turbine_controller_file' in your config");
        const TurbineControlSettingsData& ctrlSettings = *ctrlSettingsPtr;

        // ── 6a. Pitch schedule ────────────────────────────────────────────────
        //
        //   P_SOLL_KL0          | kW   → converted to [W] automatically on load
        //   MinPthAngPwr        | 0.01deg → converted to [rad] automatically on load
        //
        // StandardPitchSchedule: flat schedule — same min pitch at every power point.

        // P_SOLL_KL0 values are already in [W] — converted from kW on load.
        const std::vector<double>& p_soll_w =
            ctrlSettings.getVector("P_SOLL_KL0", "OMS", "OM-1");

        // MinPthAngPwr is already in radians — converted automatically on load
        // (raw value * factor * π/180, factor reset to 1.0, unit set to "rad").
        double minPitchRad =
            ctrlSettings.getScalar("MinPthAngPwr", "OMS", "OM-1");

        std::vector<double> pitch_at_p(p_soll_w.size(), minPitchRad);

        auto pitch_sched = std::make_unique<StandardPitchSchedule>(
            minPitchRad, p_soll_w, pitch_at_p);

        // ── 6b. Efficiency model ──────────────────────────────────────────────
        //
        //   Wirkungsgrad | - → efficiency curve [WEA]
        //
        // ConstantEfficiency: use the mean across all WEA operating points.

        const std::vector<double>& eta_vec =
            ctrlSettings.getVector("Wirkungsgrad", "WEA");

        double eta_mean = 0.0;
        for (double e : eta_vec) eta_mean += e;
        if (!eta_vec.empty())
            eta_mean /= static_cast<double>(eta_vec.size());

        auto eta_model = std::make_unique<ConstantEfficiency>(eta_mean);

        // ── 6c. Controller params ─────────────────────────────────────────────
        //
        //   LAMBDA_OPT   | -       → optimal tip-speed ratio [WEA]
        //   N_SOLL_P_OPT | 0.001rpm → factor applied on load, stored as [rpm]
        //   P_RATED      | kW      → converted to [W] automatically on load

        auto ctrl_params = TurbineControllerParams::FromConfig(turbine.get(), &sim_config);

        if (ctrlSettings.findFeature("LAMBDA_OPT"))
            ctrl_params.lambda_opt = ctrlSettings.getScalar("LAMBDA_OPT", "WEA");

        if (ctrlSettings.findFeature("N_SOLL_P_OPT"))
        {
            // N_SOLL_P_OPT is in [rpm] — numeric factor (0.001) applied on load.
            ctrl_params.n_nenn =
                ctrlSettings.getScalar("N_SOLL_P_OPT", "OMS", "OM-1");
        }

        if (ctrlSettings.findFeature("P_RATED"))
        {
            // P_RATED is already in [W] — converted from kW on load.
            ctrl_params.p_max =
                ctrlSettings.getScalar("P_RATED", "OMS", "L0");
        }

        auto controller = std::make_unique<VariableSpeedController>(
            std::move(ctrl_params),
            pitch_sched.get(),
            eta_model.get());

        auto t6 = std::chrono::steady_clock::now();
        printTiming(6, "Controller built", t5, t6,
                    "eta=" + std::to_string(eta_mean).substr(0, 5) +
                    " lambda_opt=" + std::to_string(ctrl_params.lambda_opt).substr(0, 5));

        // ── 7. Build OperationSolver ──────────────────────────────────────────
        auto op_params = OperationSolverParams::FromConfig(turbine.get(), &sim_config);

        // Wind speed vector from config range
        std::vector<double> vinf_vec;
        for (double v = sim_config.wind_speed_start();
             v <= sim_config.wind_speed_end() + sim_config.wind_speed_step() * 1e-9;
             v += sim_config.wind_speed_step()){
                vinf_vec.push_back(v);
                // TODO: debug output: std::cout << "vinf: " << v << std::endl;
             }
            

        FlowCalculatorFactory fc_factory;
        NingSolverFactory solver_factory;

        // ── Azimuth (psi) vector ──────────────────────────────────────────────
        // Config key: rotor_azimuth_psi_increment [deg]
        //   0   → scalar solve at psi = 0  (steady-state, fastest)
        //   >0  → vector [0 : step : 360)  (azimuthal average)
        const double psi_step_deg = config.getDouble("rotor_azimuth_psi_increment");
        std::vector<double> psi_vec_rad;
        if (psi_step_deg <= 0.0)
        {
            psi_vec_rad.push_back(0.0);
        }
        else
        {
            constexpr double deg2rad_psi = std::numbers::pi / 180.0;
            for (double psi_deg = 0.0;
                 psi_deg < 360.0 - psi_step_deg * 1e-9;
                 psi_deg += psi_step_deg)
                psi_vec_rad.push_back(psi_deg * deg2rad_psi);
        }

        std::cout << "  Azimuth positions: " << psi_vec_rad.size()
                  << (psi_vec_rad.size() == 1 ? " (scalar psi=0)\n" : " positions\n");

        // Collect postprocessor results per wind speed for rotor disc export.
        std::vector<BEMPostprocessResult> pp_vec;
        pp_vec.reserve(vinf_vec.size());

        // BEM callback: averages cp, ct and BEMPostprocessResult over all psi.
        BEMCallback bem_callback = [&](double vinf,
                                       double lambda,
                                       double pitch_deg)
            -> std::pair<double, double>
        {
            double rot_rate  = lambda * vinf / turbine->RotorRadius();
            double pitch_rad = pitch_deg * std::numbers::pi / 180.0;

            // ── Parallel azimuth loop ─────────────────────────────────────────
            // Each psi angle is fully independent (separate fc/solver/postproc).
            // We collect per-psi results into a pre-sized optional vector, then
            // reduce serially below — avoids complex OpenMP vector reductions.
            const int n_psi = static_cast<int>(psi_vec_rad.size());
            std::vector<std::optional<BEMPostprocessResult>> psi_results(n_psi);

            #pragma omp parallel for schedule(dynamic, 1) default(none) \
                shared(psi_results, psi_vec_rad, n_psi, \
                       turbine, sim_config, fc_factory, solver_factory, \
                       rot_rate, vinf, pitch_rad)
            for (int psi_idx = 0; psi_idx < n_psi; ++psi_idx)
            {
                const double psi = psi_vec_rad[static_cast<std::size_t>(psi_idx)];

                auto fc = fc_factory.Build(turbine.get(), rot_rate, vinf,
                                           psi, FlowModifiers{});
                auto solver = solver_factory.Build(turbine.get(), &sim_config,
                                                   fc.get(), pitch_rad, psi);

                if (!solver->Solve()) continue;

            BEMPostprocessor postproc(
                    turbine.get(), &sim_config, fc.get(),
                static_cast<double>(turbine->num_blades()));
            postproc.Process(*solver);
                if (!postproc.Success()) continue;

                psi_results[static_cast<std::size_t>(psi_idx)] = postproc.Result();
            }

            // ── Serial reduction over converged psi results ───────────────────
            BEMPostprocessResult pp_sum{};
            int n_converged = 0;

            for (int psi_idx = 0; psi_idx < n_psi; ++psi_idx)
            {
                if (!psi_results[static_cast<std::size_t>(psi_idx)].has_value())
                    continue;

                BEMPostprocessResult const &pp =
                    *psi_results[static_cast<std::size_t>(psi_idx)];

                // Accumulate scalars.
                pp_sum.cp      += pp.cp;
                pp_sum.ct      += pp.ct;
                pp_sum.p       += pp.p;
                pp_sum.thrust  += pp.thrust;
                pp_sum.torque  += pp.torque;
                pp_sum.ctorque += pp.ctorque;
                pp_sum.sum_fy  += pp.sum_fy;
                pp_sum.mx      += pp.mx;
                pp_sum.my      += pp.my;
                pp_sum.mz      += pp.mz;

                // Accumulate per-section vectors (resize on first converged psi).
                const std::size_t ns = pp.alpha_eff.size();
                auto accumVec = [&](std::vector<double> &dst,
                                    std::vector<double> const &src)
                {
                    if (dst.empty()) dst.assign(ns, 0.0);
                    for (std::size_t k = 0; k < ns && k < src.size(); ++k)
                        dst[k] += src[k];
                };
                accumVec(pp_sum.alpha_eff,              pp.alpha_eff);
                accumVec(pp_sum.cl,                     pp.cl);
                accumVec(pp_sum.cd,                     pp.cd);
                accumVec(pp_sum.cm,                     pp.cm);
                accumVec(pp_sum.local_velocity,         pp.local_velocity);
                accumVec(pp_sum.local_mach,             pp.local_mach);
                accumVec(pp_sum.local_reynolds,         pp.local_reynolds);
                accumVec(pp_sum.cp_loc,                 pp.cp_loc);
                accumVec(pp_sum.ct_loc,                 pp.ct_loc);
                accumVec(pp_sum.element_length,         pp.element_length);
                accumVec(pp_sum.element_thrust,         pp.element_thrust);
                accumVec(pp_sum.element_torque,         pp.element_torque);
                accumVec(pp_sum.element_fy,             pp.element_fy);
                accumVec(pp_sum.element_mz,             pp.element_mz);
                accumVec(pp_sum.element_airfoil_moment, pp.element_airfoil_moment);
                accumVec(pp_sum.integral_fx,            pp.integral_fx);
                accumVec(pp_sum.integral_fy,            pp.integral_fy);
                accumVec(pp_sum.integral_mx,            pp.integral_mx);
                accumVec(pp_sum.integral_my,            pp.integral_my);
                accumVec(pp_sum.integral_mz,            pp.integral_mz);

                ++n_converged;
            }

            if (n_converged == 0) return {0.0, 0.0};

            // Average all accumulated quantities over converged psi positions.
            const double inv_n = 1.0 / static_cast<double>(n_converged);
            auto scaleVec = [&](std::vector<double> &v)
            { for (auto &x : v) x *= inv_n; };

            pp_sum.cp      *= inv_n;  pp_sum.ct      *= inv_n;
            pp_sum.p       *= inv_n;  pp_sum.thrust  *= inv_n;
            pp_sum.torque  *= inv_n;  pp_sum.ctorque *= inv_n;
            pp_sum.sum_fy  *= inv_n;
            pp_sum.mx      *= inv_n;  pp_sum.my      *= inv_n;
            pp_sum.mz      *= inv_n;

            scaleVec(pp_sum.alpha_eff);              scaleVec(pp_sum.cl);
            scaleVec(pp_sum.cd);                     scaleVec(pp_sum.cm);
            scaleVec(pp_sum.local_velocity);         scaleVec(pp_sum.local_mach);
            scaleVec(pp_sum.local_reynolds);
            scaleVec(pp_sum.cp_loc);                 scaleVec(pp_sum.ct_loc);
            scaleVec(pp_sum.element_length);         scaleVec(pp_sum.element_thrust);
            scaleVec(pp_sum.element_torque);         scaleVec(pp_sum.element_fy);
            scaleVec(pp_sum.element_mz);             scaleVec(pp_sum.element_airfoil_moment);
            scaleVec(pp_sum.integral_fx);            scaleVec(pp_sum.integral_fy);
            scaleVec(pp_sum.integral_mx);            scaleVec(pp_sum.integral_my);
            scaleVec(pp_sum.integral_mz);

            // Store azimuth-averaged result for rotor disc / blade export.
            pp_vec.push_back(pp_sum);

            return {pp_sum.cp, pp_sum.ct};
        };

        OperationSolver op_solver(op_params, controller.get(), bem_callback);

        auto t7 = std::chrono::steady_clock::now();
        printTiming(7, "OperationSolver built", t6, t7,
                    std::to_string(vinf_vec.size()) + " wind speed points");

        // ── 8. Run power curve ────────────────────────────────────────────────
        auto power_curve = op_solver.Run(0.0, vinf_vec);

        auto t8 = std::chrono::steady_clock::now();
        printTiming(8, "Power curve solved", t7, t8,
                    std::to_string(vinf_vec.size()) + " wind speed points");

        // ── 9. AEP ────────────────────────────────────────────────────────────
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

        auto t9 = std::chrono::steady_clock::now();
        printTiming(9, "AEP computed", t8, t9,
                    std::to_string(vmean_vec.size()) + " mean wind speed(s)");

        // ── 11. Export simulation results ─────────────────────────────────────
        auto formatter = std::make_shared<TecplotFormatter>();
        std::unique_ptr<ISimulationResultsExporter> simExporter =
            std::make_unique<TecplotSimulationExporter>(formatter);

        // ── 10. Rotormap ──────────────────────────────────────────────────────
        {
            RotormapParams rm_params;
            rm_params.v_tip        = config.getDouble("rotormap_v_tip");
            rm_params.lambda_start = config.getDouble("rotormap_lambda_start");
            rm_params.lambda_end   = config.getDouble("rotormap_lambda_end");
            rm_params.lambda_step  = config.getDouble("rotormap_lambda_step");
            constexpr double deg2rad_rm = std::numbers::pi / 180.0;
            rm_params.pitch_start  = config.getDouble("rotormap_pitch_start") * deg2rad_rm;
            rm_params.pitch_end    = config.getDouble("rotormap_pitch_end")   * deg2rad_rm;
            rm_params.pitch_step   = config.getDouble("rotormap_pitch_step")  * deg2rad_rm;

            RotormapSolver rm_solver(turbine.get(), &sim_config);
            RotormapResult rm_result = rm_solver.Solve(rm_params);

            // Reuse the shared simExporter — ExportRotormap is part of
            // ISimulationResultsExporter, no separate exporter needed.
            if (simExporter->ExportRotormap(rm_result, "output/Rotormap.dat"))
                std::cout << "  -> " << "output/Rotormap.dat" << " written"
                          << "  (" << rm_result.count_I() << "x"
                          << rm_result.count_J() << " points)\n";
            else
                std::cerr << "  -> " << "output/Rotormap.dat" << " FAILED\n";
        }

        auto t10 = std::chrono::steady_clock::now();
        printTiming(10, "Rotormap computed", t9, t10);

        // ── 11. Blade section noise — full power curve ───────────────────────
        //  Enabled when at least one noise source is active in config.
        //  Loops over all converged operating points; produces:
        //    blade_noise_powercurve.dat — one zone per vinf, rows = sections
        auto t11_start = std::chrono::steady_clock::now();
        {
            NoiseConfig noise_cfg;
            noise_cfg.bl_tripping          = noise_bl_tripping;
            noise_cfg.bl_properties_method = noise_bl_properties_calc_method;
            noise_cfg.tbl_noise_method     = noise_tbl_noise_calc_method;
            noise_cfg.ti_noise_method      = noise_ti_noise_calc_method;
            noise_cfg.compute_bluntness    = noise_calc_blunt_te_noise;
            noise_cfg.compute_laminar      = noise_calc_lam_bl_noise;

            if (noise_cfg.any_enabled() && !pp_vec.empty())
            {
                auto noise_adapter = std::make_shared<BEMSectionNoiseAdapter>();
                SectionNoiseCalculator noise_calc(noise_cfg, noise_adapter);

                // Pre-size so each thread writes to its own slot — no mutex needed
                // on the vector itself.  Console output uses a critical section.
                //const std::size_t n_pts = pp_vec.size();
                const std::size_t n_pts = vinf_vec.size();
                std::vector<BladeNoiseResult> all_noise_results(n_pts);

                #pragma omp parallel for schedule(dynamic, 1) default(none) \
                    shared(all_noise_results, pp_vec, vinf_vec, \
                           noise_calc, turbine, sim_config, n_pts, \
                           std::cout)
                for (std::size_t j = 0; j < n_pts; ++j)
                {
                    BEMPostprocessResult const &pp_j = pp_vec[j];
                    const double vinf_j = vinf_vec[j];

                    // Each iteration is independent — Calculate() is const
                    // and reads only its own inputs.
                    all_noise_results[j] = noise_calc.Calculate(
                        pp_j, turbine.get(), sim_config,
                        vinf_j,
                        pp_j.local_velocity,
                        pp_j.local_mach,
                        pp_j.local_reynolds);

                    #pragma omp critical
                    {
                    std::cout << "  [noise] v_inf = " << std::fixed
                              << std::setprecision(1) << vinf_j << " m/s"
                              << "  (" << (j + 1) << "/" << n_pts << ")\n";
                }
                }

                std::unique_ptr<INoiseResultsExporter> noiseExporter =
                    std::make_unique<TecplotNoiseExporter>(formatter);

                // Full power-curve noise file (one zone per operating point)
                if (noiseExporter->ExportPowerCurveNoise(
                        all_noise_results, "output/blade_noise_powercurve.dat"))
                    std::cout << "  -> output/blade_noise_powercurve.dat written"
                              << "  (" << all_noise_results.size() << " zones, "
                              << (all_noise_results.empty() ? 0
                                  : all_noise_results[0].sections.size())
                              << " sections each)\n";
                else
                    std::cerr << "  -> output/blade_noise_powercurve.dat FAILED\n";
            }
            else if (!noise_cfg.any_enabled())
            {
                std::cout << "  Noise calculation skipped "
                             "(all noise sources disabled in config)\n";
            }
        }
        auto t11 = std::chrono::steady_clock::now();
        printTiming(11, "Blade noise (power curve)", t11_start, t11,
                    std::to_string(pp_vec.size()) + " operating points");

        // turbine_performance.dat — power curve, one row per wind speed
        if (simExporter->ExportPowerCurve(power_curve, "output/turbine_performance.dat"))
            std::cout << "  -> output/turbine_performance.dat written\n";
        else
            std::cerr << "  -> output/turbine_performance.dat FAILED\n";

        // blade_data.dat — section loads at the rated operating point
        if (!pp_vec.empty() && !power_curve.empty())
        {
            // Pick operating point with highest electrical power (≈ rated).
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
            // Guard: pp_vec may be shorter than power_curve if any point
            // failed to converge and was skipped in the callback.
            std::size_t blade_idx = std::min(rated_idx, pp_vec.size() - 1);

            if (simExporter->ExportBladeData(pp_vec[blade_idx], turbine.get(),
                                             vinf_vec[blade_idx], "output/blade_data.dat"))
                std::cout << "  -> output/blade_data.dat written"
                          << "  (v_inf = " << vinf_vec[blade_idx] << " m/s)\n";
            else
                std::cerr << "  -> output/blade_data.dat FAILED\n";
        }

        // rotor_disc_data.dat — section loads at every wind speed, one zone each
        if (!pp_vec.empty())
        {
            if (simExporter->ExportRotorDiscData(pp_vec, turbine.get(),
                                                 vinf_vec, "output/rotor_disc_data.dat"))
                std::cout << "  -> output/rotor_disc_data.dat written"
                          << "  (" << pp_vec.size() << " zones)\n";
            else
                std::cerr << "  -> output/rotor_disc_data.dat FAILED\n";
        }

        auto t12 = std::chrono::steady_clock::now();
        printTiming(12, "Results exported", t10, t12, "4 files");

        // ── Summary ───────────────────────────────────────────────────────────
        double total_ms = std::chrono::duration<double, std::milli>(
                              t12 - t_total_start)
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
