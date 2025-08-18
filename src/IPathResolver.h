#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

/**
 * @brief Interface for path resolution strategies
 *
 * IPathResolver provides an abstraction layer for converting input path
 * strings into resolved filesystem paths. Enables different path handling
 * strategies through polymorphism for flexible file organization.
 *
 * ## Key Features
 * - **Strategy Pattern**: Multiple resolution implementations (default, flexible)
 * - **Path Abstraction**: Isolates path logic from business logic
 * - **Flexible Organization**: Different approaches to file/directory structure
 * - **Input Validation**: Implementations handle various input path formats
 *
 * ## Common Implementations
 * - **DefaultPathResolver**: Restrictive, uses default directory for most inputs
 * - **FlexiblePathResolver**: Preserves relative path structure under base directory
 * - **AbsolutePathResolver**: Forces absolute path usage
 *
 * @see DefaultPathResolver, FlexiblePathResolver for concrete implementations
 * @see DataExporter for typical usage in export systems
 *
 * @example
 * ```cpp
 * std::unique_ptr<IPathResolver> resolver =
 *     std::make_unique<FlexiblePathResolver>("/output");
 *
 * fs::path resolved = resolver->resolvePath("data/results");
 * ```
 */
class IPathResolver {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IPathResolver() = default;

    /**
     * @brief Resolves input path string to filesystem path
     * @param inputPath Input path string (empty, relative, or absolute)
     * @return Resolved filesystem path according to strategy
     */
    virtual fs::path resolvePath(const std::string& inputPath) const = 0;

};
