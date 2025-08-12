#pragma once

#include <string>
#include <vector>
#include <filesystem>

// Airfoil Geometry File List Data Structure
struct AirfoilGeometryFileInfo {
    std::string filePath;
    std::string fileName;
    bool exists;

    AirfoilGeometryFileInfo(const std::string& path) : filePath(path) {
        fileName = std::filesystem::path(path).filename().string();
        exists = std::filesystem::exists(path);
    }
};