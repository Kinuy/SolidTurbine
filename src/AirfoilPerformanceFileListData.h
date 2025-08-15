#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "IStructuredData.h"
#include "AirfoilPerformanceFileInfo.h"

class AirfoilPerformanceFileListData : public IStructuredData {
private:
    std::vector<AirfoilPerformanceFileInfo> fileInfos;
    std::vector<std::string> headers;
    std::string revision;
    std::string date;

public:
    void addHeader(const std::string& header);

    void addFilePath(const std::string& filePath);

    const std::vector<AirfoilPerformanceFileInfo>& getFileInfos() const;
    const std::vector<std::string>& getHeaders() const;
    const std::string& getRevision() const;
    const std::string& getDate() const;

    std::string getTypeName() const override;
    size_t getRowCount() const override;

    // Convenience methods
    std::vector<std::string> getValidFilePaths() const;

    std::vector<std::string> getMissingFiles() const;

    size_t getValidFileCount() const;
};

