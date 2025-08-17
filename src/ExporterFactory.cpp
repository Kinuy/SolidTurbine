#include "ExporterFactory.h"


std::unique_ptr<IExporter> ExporterFactory::createExporter(
    PathStrategy strategy,
    const std::string& defaultDirectory
) {
    std::unique_ptr<IPathResolver> pathResolver;

    switch (strategy) {
    case PathStrategy::DEFAULT:
        pathResolver = std::make_unique<DefaultPathResolver>(defaultDirectory);
        break;
    case PathStrategy::FLEXIBLE:
        pathResolver = std::make_unique<FlexiblePathResolver>(defaultDirectory);
        break;
    }

    auto directoryManager = std::make_unique<FileSystemDirectoryManager>();
    auto fileWriter = std::make_unique<StandardFileWriter>();

    return std::make_unique<DataExporter>(
        std::move(pathResolver),
        std::move(directoryManager),
        std::move(fileWriter)
    );
}