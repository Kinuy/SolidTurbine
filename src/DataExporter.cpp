#include "DataExporter.h"

DataExporter::DataExporter(
    std::unique_ptr<IPathResolver> pathResolver,
    std::unique_ptr<IDirectoryManager> directoryManager,
    std::unique_ptr<IFileWriter> fileWriter
) : pathResolver(std::move(pathResolver)),
directoryManager(std::move(directoryManager)),
fileWriter(std::move(fileWriter)) {
}

bool DataExporter::exportData(const std::string& path, const std::string& filename, const std::string& data) {
    try {
        // Resolve the export path
        fs::path exportPath = pathResolver->resolvePath(path);

        // Ensure directory exists
        if (!directoryManager->ensureDirectoryExists(exportPath)) {
            return false;
        }

        // Write the file
        fs::path fullPath = exportPath / filename;
        return fileWriter->writeFile(fullPath, data);

    }
    catch (const std::exception& e) {
        std::cerr << "Export failed: " << e.what() << std::endl;
        return false;
    }
}
