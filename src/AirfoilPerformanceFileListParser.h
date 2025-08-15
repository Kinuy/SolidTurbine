#pragma once


#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <memory>

#include "IFileListParser.h"
#include "IStructuredData.h"
#include "AirfoilPerformanceFileListData.h"

// Concrete implementation of File List Parser for airfoil performance file lists
class AirfoilPerformanceFileListParser : public IFileListParser {

private:

    std::string trimLine(const std::string& line) const;

    bool isAbsolutePath(const std::string& path) const;

    std::string resolveRelativePath(const std::string& relativePath, const std::string& indexFilePath) const;

public:

    std::unique_ptr<IStructuredData> parseFileList(const std::string& indexFilePath) override;

    std::vector<std::string> getSupportedExtensions() const override;

};

