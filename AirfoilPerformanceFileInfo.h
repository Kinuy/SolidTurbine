#pragma once

#include <string>
#include <filesystem>

// Responsible for airfoil performance file list data structure
struct AirfoilPerformanceFileInfo {
    std::string filePath;
    std::string fileName;
    bool exists;

    AirfoilPerformanceFileInfo(const std::string& path) : filePath(path) {
        fileName = std::filesystem::path(path).filename().string();
        exists = std::filesystem::exists(path);
    }
};