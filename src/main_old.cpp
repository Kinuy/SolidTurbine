#include <iostream>
#include <memory>

#include "ConfigurationSchema.h"
#include "FileReader.h"
#include "ConfigurationParser.h"
#include "ExporterFactory.h"
#include "DXFBlade3D.h"
#include "TurbineGeometry.h"

// ── Simulation layer ──────────────────────────────────────────────────────────
#include "ConfigurationAdapter.h" // Configuration  → ISimulationConfig
#include "FlowCalculatorFactory.h"
#include "NingSolverFactory.h"
#include "VariableSpeedController.h"
#include "StandardPitchSchedule.h"
#include "EfficiencyModels.h"
#include "OperationSolver.h"
#include "AEPCalculator.h"
#include "BEMPostprocessor.h"

/**
 * @brief Dummy Function to keep window after run open
 */
static void waitForKeyPress()
{
	std::cout << "Press Enter to continue...";
	std::cin.get();
}

/**
 * @brief Programm to simulate a given turbine using SOLID principles
 */
int main(int /*argc*/, char **argv)
{

	try
	{
		// ── 1. Schema — keys already present + new ones for solver/controller ──
		ConfigurationSchema schema;
		// Project data
		schema.addString("project_name", true, "Name of project");
		schema.addString("project_id", true, "Project number for identification");
		schema.addString("project_revision", true, "Project revision");
		schema.addString("project_date", true, "Date of simulation run");
		schema.addString("project_engineer", true, "Engineer responsible for simulation");
		// Data tables -> airfoil performance, airfoil geometry , blade geometry
		schema.addDataFile("airfoil_geometry_files_file", true, "Path to airfoil geometry data file list");
		schema.addDataFile("airfoil_performance_files_file", true, "Path to airfoil performance data file list");
		schema.addDataFile("blade_geometry_file", true, "Path to blade geometry data");
		// Turbine data
		schema.addBool("turbine_is_horizontal", true, "flag for horizontal or vertical turbine");
		schema.addInt("number_of_blades", true, "blade number of turbine");
		schema.addDouble("hub_radius", true, "turbine hub radius [m]");
		schema.addDouble("cone", true, "cone angle [deg]");
		schema.addDouble("yaw", true, "yaw angle [deg]");
		schema.addDouble("tilt", true, "tilt angle [deg]");
		schema.addDouble("tower_dist", true, "tower distance [m]");
		schema.addDouble("hub_height", true, "hub height [m]");
		// Operation data
		schema.addDouble("rated_rotorspeed", true, "rotor speed at rated conditions [rpm]");
		schema.addDouble("min_rotorspeed", true, "cut in min rotor speed [rpm]");
		schema.addDouble("rated_electrical_power", true, "rated electrical power of turbine [W]");
		// Simulation data
		schema.addBool("simulation_is_time_based", true, "flag for static or time based simulation");
		schema.addRange("wind_speed_range", "windspeed_start", "windspeed_end", "windspeed_step", true, "wind speed range for static simulation [m/s]");
		schema.addRange("tip_speed_ratio_range", "tsr_start", "tsr_end", "tsr_step", true, "tip speed ratio range for static simulation [-]");
		schema.addRange("pitch_angle_range", "pitch_start", "pitch_emd", "pitch_step", true, "pitch angle range for static simulation [deg]");

		// ── NEW keys required by solver and controller ────────────────────────
		// Add these to your input file as well.
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
		schema.addDouble("aep_k_factor", true, "Weibull k factor for AEP");
		schema.addDouble("aep_money_kwh", true, "Energy price [EUR/kWh]");
		schema.addRange("aep_vmean_range",
						"vmean_start", "vmean_end", "vmean_step",
						true, "Mean wind speed range for AEP [m/s]");

		// ── 2. Parse ──────────────────────────────────────────────────────────
		auto fileReader = std::make_unique<FileReader>(argv[1]);
		ConfigurationParser parser(std::move(schema), std::move(fileReader));
		Configuration config = parser.parse();

		// Get Blade data
		[[maybe_unused]] const BladeGeometryData *bladeGeometry = config.getBladeGeometry();

		// Get airfoil perfo and geo file list -> processes internally each file in list
		[[maybe_unused]] const AirfoilPerformanceFileListData *airfoilPerformanceFileList = config.getAirfoilPerformanceFileList();
		[[maybe_unused]] const AirfoilGeometryFileListData *airfoilGeometryFileList = config.getAirfoilGeometryFileList();

		// ── 3. Build TurbineGeometry (your existing code, unchanged) ──────────
		// Interpolate Airfoil Geometries and performances on blade sections
		std::unique_ptr<BladeInterpolator> bladeInterpolator = config.createBladeInterpolator();

		// Write DXF file with sample data
		// auto dxfBlade3D = std::make_unique<DXFBlade3D>(std::move(bladeInterpolator));
		// Export 3D section data to custom text file
		// auto exporter = ExporterFactory::createExporter();
		// exporter->exportData("", "BladeSections.txt", dxfBlade3D->dataToString());

		// Use configuration and pas to objects with type safety
		[[maybe_unused]] double ratedRotorSpeed = config.getDouble("rated_rotorspeed");
		[[maybe_unused]] int numberOfBlades = config.getInt("number_of_blades");
		[[maybe_unused]] bool isHorizontal = config.getBool("turbine_is_horizontal");
		[[maybe_unused]] bool isTimeBased = config.getBool("simulation_is_time_based");

		std::cout << "Configuration loaded successfully:" << std::endl;

		// Create TurbineGeometry
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

		// ── 4. Wrap Configuration as ISimulationConfig ────────────────────────
		//
		// This is the only adapter you need.  From here on, pass turbine.get()
		// and &sim_config everywhere — no more InputCase, no polars vector.
		//
		ConfigurationAdapter sim_config(config);

		// ── 5. Build controller ───────────────────────────────────────────────
		//
		// TurbineControllerParams::FromConfig reads everything from the two
		// domain objects in one call — replaces all opermot_inputdata getters.
		//
		auto ctrl_params = TurbineControllerParams::FromConfig(
			turbine.get(), &sim_config);

		// Pitch schedule: pmin breakpoints and delta-alpha from config or hardcoded.
		// Extend with schema.addRange("pitch_schedule",...) if you want file-driven.
		auto pitch_sched = std::make_unique<StandardPitchSchedule>(
			0.0,					   // base pitch [rad]
			std::vector<double>{0.0},  // P_min breakpoints [W]
			std::vector<double>{0.0}); // delta-alpha values [rad]

		auto eta_model = std::make_unique<ConstantEfficiency>(0.94);

		auto controller = std::make_unique<VariableSpeedController>(
			std::move(ctrl_params),
			pitch_sched.get(),
			eta_model.get());

		// ── 6. Build OperationSolver ──────────────────────────────────────────
		//
		// OperationSolverParams::FromConfig reads air_density, rotor_radius,
		// p_max from the same two domain objects.
		//
		auto op_params = OperationSolverParams::FromConfig(
			turbine.get(), &sim_config);

		// Wind speed vector built from config range keys
		std::vector<double> vinf_vec;
		for (double v = sim_config.wind_speed_start();
			 v <= sim_config.wind_speed_end() + sim_config.wind_speed_step() * 1e-9;
			 v += sim_config.wind_speed_step())
			vinf_vec.push_back(v);

		// BEM callback — captures turbine + sim_config, creates a fresh
		// FlowCalculator + NingSolver for each (vinf, lambda, pitch) call.
		FlowCalculatorFactory fc_factory;
		NingSolverFactory solver_factory;
		const double psi = 0.0; // steady-state: single azimuth

		BEMCallback bem_callback = [&](double vinf,
									   double lambda,
									   double pitch_deg)
			-> std::pair<double, double>
		{
			double rot_rate = lambda * vinf / turbine->RotorRadius();
			double pitch_rad = pitch_deg * M_PI / 180.0;

			auto fc = fc_factory.Build(
				turbine.get(), rot_rate, vinf, psi, FlowModifiers{});

			auto solver = solver_factory.Build(
				turbine.get(), &sim_config, fc.get(), pitch_rad, psi);

			if (!solver->Solve())
				return {0.0, 0.0};

			// ── Postprocess ────────────────────────────────────────────────
			BEMPostprocessor postproc(
				turbine.get(),
				&sim_config,
				fc.get(),
				static_cast<double>(turbine->num_blades()));

			postproc.Process(*solver);

			if (!postproc.Success())
				return {0.0, 0.0};

			BEMPostprocessResult const &pp = postproc.Result();

			// Return rotor-integrated Cp and Ct from postprocessor
			// (these are computed from actual blade loads, not BEM induction)
			return {pp.cp, pp.ct};
		};

		OperationSolver op_solver(op_params, controller.get(), bem_callback);

		// ── 7. Run power curve ────────────────────────────────────────────────
		auto power_curve = op_solver.Run(0.0, vinf_vec);

		// ── 8. AEP ────────────────────────────────────────────────────────────
		std::vector<double> pel_vec;
		for (auto const &pt : power_curve)
			pel_vec.push_back(pt.p_el);

		AEPCalculator aep_calc(
			vinf_vec, pel_vec,
			sim_config.wind_speed_bin_width(),
			sim_config.weibull_k(),
			sim_config.energy_price_per_kwh());

		auto vmean_vec = sim_config.mean_wind_speeds();
		auto aep_results = aep_calc.Compute(vmean_vec[0]);

		std::cout << "Simulation complete.\n";
	}
	catch(const std::exception& e){
		std::cerr << "Configuration error: " << e.what() << std::endl;
		waitForKeyPress();
		return 1;
	}

	waitForKeyPress();
	return 0;
}