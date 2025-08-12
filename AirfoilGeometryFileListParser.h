#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "IFileListParser.h"
#include "AirfoilGeometryFileListData.h"
#include "IStructuredData.h"


// Responsible for parsing airfoil geometry file lists
class AirfoilGeometryFileListParser : public IFileListParser {

private:

    std::string trimLine(const std::string& line) const;

    bool isAbsolutePath(const std::string& path) const;

    std::string resolveRelativePath(const std::string& relativePath, const std::string& indexFilePath) const;

public:

    std::unique_ptr<IStructuredData> parseFileList(const std::string& indexFilePath) override;

    std::vector<std::string> getSupportedExtensions() const override;

};

