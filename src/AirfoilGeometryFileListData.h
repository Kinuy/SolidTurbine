#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "IStructuredData.h"
#include "AirfoilGeometryFileInfo.h"
#include "AirfoilGeometryFileListParser.h"


// Responsible for storing airfoil geometry file information
class AirfoilGeometryFileListData : public IStructuredData {

private:

    std::vector<AirfoilGeometryFileInfo> fileInfos;
    std::vector<std::string> headers;
    std::string revision;
    std::string date;

public:

    void addHeader(const std::string& header);

    void addFilePath(const std::string& filePath);

    const std::vector<AirfoilGeometryFileInfo>& getFileInfos() const;

    const std::vector<std::string>& getHeaders() const;

    const std::string& getRevision() const;

    const std::string& getDate() const;

    std::string getTypeName() const override;

    size_t getRowCount() const override;

    std::vector<std::string> getValidFilePaths() const;

    std::vector<std::string> getMissingFiles() const;

    size_t getValidFileCount() const;

    AirfoilGeometryFileInfo getFileByName(const std::string& airfoilName) const;

};

