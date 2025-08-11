#pragma once

#include "IDataFileParser.h"
#include "IStructuredData.h"
#include "BladeGeometryData.h"

#include <sstream>
#include <fstream>

// Tabular data parser
class BladeGeometryParser : public IDataFileParser {
private:
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter) const;

public:
    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath);

    std::vector<std::string> getSupportedExtensions() const;
};

