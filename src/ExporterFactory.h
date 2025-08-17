#pragma once


#include <memory>
#include <string>

#include "PathStrategy.h"
#include "IExporter.h"
#include "IPathResolver.h"
#include "DefaultPathResolver.h"
#include "FlexiblePathResolver.h"
#include "DataExporter.h"
#include "FileSystemDirectoryManager.h"
#include "StandardFileWriter.h"

// Responsible for Exporter creation
class ExporterFactory {

public:

    static std::unique_ptr<IExporter> createExporter(
        PathStrategy strategy = PathStrategy::DEFAULT,
        const std::string& defaultDirectory = "export"
    );
};