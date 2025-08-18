#include <iostream>
#include <memory>

#include "ConfigurationSchema.h"
#include "FileReader.h"
#include "ConfigurationParser.h"
#include "ExporterFactory.h"


// Programm to simulate a given turbine using SOLID principles
int main(int argc , char** argv){

	
	try {
		// Define simulation schema
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
		// Operation data
		schema.addDouble("rated_rotorspeed", true, "rotor speed at rated conditions [rpm]");
		schema.addDouble("min_rotorspeed", true, "cut in min rotor speed [rpm]");
		schema.addDouble("rated_electrical_power", true, "rated electrical power of turbine [W]");
		// Simulation data
		schema.addBool("simulation_is_time_based",true, "flag for static or time based simulation");
		schema.addRange("wind_speed_range","windspeed_start","windspeed_end","windspeed_step", true, "wind speed range for static simulation [m/s]");
		schema.addRange("tip_speed_ratio_range","tsr_start","tsr_end","tsr_step", true, "tip speed ratio range for static simulation [-]");
		schema.addRange("pitch_angle_range","pitch_start","pitch_emd","pitch_step", true, "pitch angle range for static simulation [deg]");

		// Create parser
		auto fileReader = std::make_unique<FileReader>("ProjectData.dat");
		ConfigurationParser parser(std::move(schema), std::move(fileReader));

		// Parse configuration
		Configuration config = parser.parse();

		// Get Blade data
		const BladeGeometryData* bladeGeometry = config.getBladeGeometry();
		
		// Get airfoil perfo and geo file list -> processes internally each file in list
		const AirfoilPerformanceFileListData* airfoilPerformanceFileList = config.getAirfoilPerformanceFileList();
		const AirfoilGeometryFileListData* airfoilGeometryFileList = config.getAirfoilGeometryFileList();

		// Interpolate Airfoil Geometries and performances on blade sections
		std::unique_ptr<BladeGeometryInterpolator> bladeInterpolator = config.createBladeInterpolator();
		std::vector<InterpolatedBladeSection> InterpolatedSection = bladeInterpolator->interpolateAllSections();

		// Export file
		auto exporter = ExporterFactory::createExporter();
		exporter->exportData("", "basic_test.txt", "Basic export data\n");

		// Use configuration and pas to objects with type safety
		double ratedRotorSpeed = config.getDouble("rated_rotorspeed");
		int numberOfBlades = config.getInt("number_of_blades");
		bool isHorizontal = config.getBool("turbine_is_horizontal");
		bool isTimeBased = config.getBool("simulation_is_time_based");

		std::cout << "Configuration loaded successfully:" << std::endl;

		// Create simulation setup


	}
	catch(const std::exception& e){
		std::cerr << "Configuration error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}