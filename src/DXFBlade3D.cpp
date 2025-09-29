#include "DXFBlade3D.h"

DXFBlade3D::DXFBlade3D(std::unique_ptr<BladeInterpolator> bI) : bladeInterpolator(std::move(bI))
{
	dxfFilePath = "blade3D.dxf";

	bladeSections = bladeInterpolator->getBladeSections();
	
	dxfPointList.clear();

	createDXFModel();

	writeDXFFile();
}

DXFBlade3D::~DXFBlade3D()
{
}

void DXFBlade3D::createDXFModel()
{
	fillDxfPointList();

	fillDxfPolyLineList();
}

void DXFBlade3D::fillDxfPointList()
{
	// iterate over sections
	for (int s = 0; s < bladeSections.size(); s++) {
		// iterate over coordinates in each section
		auto bladeSectionsCoord = bladeSections.at(s)->airfoilGeometry->getCoordinates();
		for (int c = 0; c < bladeSectionsCoord.size(); c++) {

			double sectionScale = bladeSections.at(s)->chord;
			double xScale = bladeSectionsCoord.at(c).x * sectionScale;
			double yScale = bladeSectionsCoord.at(c).y * sectionScale;
			double zScale = bladeSections.at(s)->bladeRadius;
			// fill list with points
			dxfPointList.emplace_back(xScale, yScale, zScale);
		}
	}
}

void DXFBlade3D::fillDxfPolyLineList()
{
	// iterate over sections
	for (int s = 0; s < bladeSections.size(); s++) {
		// Create temp list for all points of one airfoil
		std::vector<DXFPoint3D> airfoilPolyPoints;

		// iterate over coordinates in each section
		auto bladeSectionsCoord = bladeSections.at(s)->airfoilGeometry->getCoordinates();
		for (int c = 0; c < bladeSectionsCoord.size(); c++) {

			double sectionScale = bladeSections.at(s)->chord;
			double xScale = bladeSectionsCoord.at(c).x * sectionScale;
			double yScale = bladeSectionsCoord.at(c).y * sectionScale;
			double zScale = bladeSections.at(s)->bladeRadius;
			// fill list with points
			airfoilPolyPoints.emplace_back(xScale, yScale, zScale);
		}
		dxfPolyLineList.push_back(airfoilPolyPoints);
	}
}

void DXFBlade3D::writeDXFFile() const
{
	try {
		auto fileWriter = std::make_unique<DXFFileWriter>(dxfFilePath);
		DXFDocument document(std::move(fileWriter));

		// Add entities
		for (const auto& point : dxfPointList) {
			document.addPoint(point, DXFColor(2)); // Green points for blade geometry
		}

		for (const auto& polyLine : dxfPolyLineList) {
			document.addPolyLine(polyLine, true, DXFColor(4)); // blue points for blade geometry
		}


		//document.addPolyLine(airfoilPolyPoints, true, DXFColor(1));	// Magenta closed polyline

		std::cout << "DXF file 'Blade3D.dxf' created successfully!" << std::endl;

	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

std::string DXFBlade3D::dataToString() const
{
	std::string dataString;
	// iterate over sections
	for (const auto& section : bladeSections) {
		dataString += "NAME\t" + section->airfoilName + "\n";
		dataString += "RELTHICK[%]\t" + std::to_string(section->relativeThickness) + "\n";
		dataString += "RADIUS[m]\t" + std::to_string(section->bladeRadius) + "\n";
		dataString += "CHORD[m]\t" + std::to_string(section->chord) + "\n";
		dataString += "#\tX\tY\tZ\n";
		// iterate over coordinates in each section
		for(const auto& coord : section->airfoilGeometry->getCoordinates()) {
			//dataString += section->type + "\t" + std::to_string(coord.x) + "\t" + std::to_string(coord.y) + "\t" + std::to_string(coord.z) + "\n";
			dataString += section->type + "\t" + std::to_string(coord.x) + "\t" + std::to_string(coord.y) + "\n";
		}
		dataString += "\n";
	}

	return dataString;
}
