#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "IDataFileParser.h"
#include "AirfoilGeometryData.h"
#include "AirfoilCoordinate.h"
#include "AirfoilMarker.h"
#include "AirfoilGeometryFileInfo.h"


// Airfoil Geometry File Parser
class AirfoilGeometryParser : public IDataFileParser {

private:

    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter) const;

    bool isCoordinateLine(const std::vector<std::string>& tokens) const;

    bool isMarkerLine(const std::vector<std::string>& tokens) const;

    bool isHeaderLine(const std::vector<std::string>& tokens) const;

public:

    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) override;

    std::vector<std::string> getSupportedExtensions() const override;

};
