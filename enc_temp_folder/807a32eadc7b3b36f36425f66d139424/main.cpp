#include <iostream>
#include <memory>

#include "ConfigurationSchema.h"
#include "FileReader.h"
#include "ConfigurationParser.h"


// Programm to simulate a given turbine using SOLID principles
int main(int argc , char** argv){

	
	try {
		// Define simulation schema
		ConfigurationSchema schema;
		schema.addDouble("rated_rotorspeed", true, "rotor speed at rated conditions in rpm");
		schema.addDouble("number_of_blades", true, "blade number of turbine");
		schema.addBool("turbine_is_horizontal", true, "flag for horizontal or vertical turbine");
		schema.addBool("simulation_is_time_based",true, "flag for static or time based simulation");
		schema.addString("airfoil_performance_file", true, "path to file that stores paths to airfoil perfo files)");
		schema.addString("airfoil_geometry_file", true, "path to file that stores paths to airfoil geo files)");

		// Create parser
		auto fileReader = std::make_unique<FileReader>("ProjectData.dat");
		ConfigurationParser parser(std::move(schema), std::move(fileReader));

		// Parse configuration
		Configuration config = parser.parse();

		// Use configuration with type safety
		double ratedSpeed = config.getDouble("rated_rotorspeed");
		int numberOfBlades = config.getInt("number_of_blades");
		bool isHorizontal = config.getBool("turbine_is_horizontal");
		bool isTimeBased = config.getBool("simulation_is_time_based");
		std::string airfoilPerfoFile = config.getString("airfoil_perfo_file");
		std::string airfoilGeoFile = config.getString("airfoil_geo_file");
		std::cout << "Configuration loaded successfully:" << std::endl;

	}
	catch(const std::exception& e){
		std::cerr << "Configuration error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}