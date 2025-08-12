#pragma once

#include<vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <algorithm>

#include "AirfoilPerformanceData.h"
#include "IDataFileParser.h"


// Airfoil Performance File Parser (Individual .dat files)
class AirfoilPerformanceParser : public IDataFileParser {

private:

    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter = '\t') const;

    bool isHeaderLine(const std::vector<std::string>& tokens) const;

    bool isDataLine(const std::vector<std::string>& tokens) const;

public:

    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) override;

    std::vector<std::string> getSupportedExtensions() const override;
};

