#pragma once

#include <filesystem>
#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "IExporter.h"
#include "IPathResolver.h"
#include "IDirectoryManager.h"
#include "IFileWriter.h"

namespace fs = std::filesystem;



// Responsible for coordinating export operations
class DataExporter : public IExporter {
private:
    std::unique_ptr<IPathResolver> pathResolver;
    std::unique_ptr<IDirectoryManager> directoryManager;
    std::unique_ptr<IFileWriter> fileWriter;

public:
    DataExporter(
        std::unique_ptr<IPathResolver> pathResolver,
        std::unique_ptr<IDirectoryManager> directoryManager,
        std::unique_ptr<IFileWriter> fileWriter
    );

    bool exportData(const std::string& path, const std::string& filename, const std::string& data) override;
};

