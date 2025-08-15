#include "AirfoilPerformanceFileListData.h"


void AirfoilPerformanceFileListData::addHeader(const std::string& header) {
    headers.push_back(header);

    // Parse special headers
    if (header.find("Revision") != std::string::npos) {
        auto pos = header.find_last_of('\t');
        if (pos != std::string::npos && pos + 1 < header.length()) {
            revision = header.substr(pos + 1);
        }
    }
    else if (header.find("Date") != std::string::npos) {
        auto pos = header.find_last_of('\t');
        if (pos != std::string::npos && pos + 1 < header.length()) {
            date = header.substr(pos + 1);
        }
    }
}

void AirfoilPerformanceFileListData::addFilePath(const std::string& filePath) {
    fileInfos.emplace_back(filePath);
}

const std::vector<AirfoilPerformanceFileInfo>& AirfoilPerformanceFileListData::getFileInfos() const { return fileInfos; }
const std::vector<std::string>& AirfoilPerformanceFileListData::getHeaders() const { return headers; }
const std::string& AirfoilPerformanceFileListData::getRevision() const { return revision; }
const std::string& AirfoilPerformanceFileListData::getDate() const { return date; }

std::string AirfoilPerformanceFileListData::getTypeName() const { return "AirfoilPerformanceFileList"; }
size_t AirfoilPerformanceFileListData::getRowCount() const { return fileInfos.size(); }

// Convenience methods
std::vector<std::string> AirfoilPerformanceFileListData::getValidFilePaths() const {
    std::vector<std::string> validPaths;
    for (const auto& info : fileInfos) {
        if (info.exists) {
            validPaths.push_back(info.filePath);
        }
    }
    return validPaths;
}

std::vector<std::string> AirfoilPerformanceFileListData::getMissingFiles() const {
    std::vector<std::string> missingPaths;
    for (const auto& info : fileInfos) {
        if (!info.exists) {
            missingPaths.push_back(info.filePath);
        }
    }
    return missingPaths;
}

size_t AirfoilPerformanceFileListData::getValidFileCount() const {
    return std::count_if(fileInfos.begin(), fileInfos.end(),
        [](const AirfoilPerformanceFileInfo& info) { return info.exists; });
}