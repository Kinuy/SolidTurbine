#include "AirfoilGeometryFileListData.h"


void AirfoilGeometryFileListData::addHeader(const std::string& header) {
    headers.push_back(header);

    // Parse special headers (same format as performance files)
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

void AirfoilGeometryFileListData::addFilePath(const std::string& filePath) {
    fileInfos.emplace_back(filePath);
}

const std::vector<AirfoilGeometryFileInfo>& AirfoilGeometryFileListData::getFileInfos() const { return fileInfos; }

const std::vector<std::string>& AirfoilGeometryFileListData::getHeaders() const { return headers; }

const std::string& AirfoilGeometryFileListData::getRevision() const { return revision; }

const std::string& AirfoilGeometryFileListData::getDate() const { return date; }

std::string AirfoilGeometryFileListData::getTypeName() const { return "AirfoilGeometryFileList"; }

size_t AirfoilGeometryFileListData::getRowCount() const { return fileInfos.size(); }

std::vector<std::string> AirfoilGeometryFileListData::getValidFilePaths() const {
    std::vector<std::string> validPaths;
    for (const auto& info : fileInfos) {
        if (info.exists) {
            validPaths.push_back(info.filePath);
        }
    }
    return validPaths;
}

std::vector<std::string> AirfoilGeometryFileListData::getMissingFiles() const {
    std::vector<std::string> missingPaths;
    for (const auto& info : fileInfos) {
        if (!info.exists) {
            missingPaths.push_back(info.filePath);
        }
    }
    return missingPaths;
}

size_t AirfoilGeometryFileListData::getValidFileCount() const {
    return std::count_if(fileInfos.begin(), fileInfos.end(),
        [](const AirfoilGeometryFileInfo& info) { return info.exists; });
}

AirfoilGeometryFileInfo AirfoilGeometryFileListData::getFileByName(const std::string& airfoilName) const {
    auto it = std::find_if(fileInfos.begin(), fileInfos.end(),
        [&airfoilName](const AirfoilGeometryFileInfo& info) {
            return info.fileName.find(airfoilName) != std::string::npos;
        });

    if (it == fileInfos.end()) {
        throw std::runtime_error("No geometry file found for airfoil: " + airfoilName);
    }
    return *it;
}