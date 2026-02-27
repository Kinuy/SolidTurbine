#pragma once

#include <filesystem>
#include <string>

#include "IPathResolver.h"

namespace fs = std::filesystem;

/**
 * @brief Flexible path resolver that preserves relative path structure
 *
 * FlexiblePathResolver implements IPathResolver with adaptive path handling
 * that combines user-specified relative paths with a default base directory.
 * Unlike restrictive resolvers, this implementation respects relative path
 * structure while providing consistent output organization.
 *
 * ## Resolution Strategy
 * - **Empty paths**: Resolved to `current_working_directory / defaultDirectory`
 * - **Absolute paths**: Preserved unchanged (user has explicit control)
 * - **Relative paths**: Combined as `current_working_directory / defaultDirectory / inputPath`
 *
 * ## Key Advantages
 * - **Path Preservation**: Maintains user-specified directory structures
 * - **Flexible Organization**: Enables subdirectory creation within base directory
 * - **User Control**: Respects relative path intentions while providing safe defaults
 * - **Predictable Base**: All output organized under configured default directory
 *
 * ## Use Cases
 * - Applications requiring user-controlled output organization
 * - Systems where relative paths represent meaningful structure
 * - Flexible export systems with user-defined subdirectories
 * - Development environments with dynamic path requirements
 *
 * @see IPathResolver for the base interface
 * @see DefaultPathResolver for more restrictive alternative
 *
 * @example
 * ```cpp
 * FlexiblePathResolver resolver("output");
 *
 * // Empty input -> /current/working/dir/output
 * auto path1 = resolver.resolvePath("");
 *
 * // Relative input -> /current/working/dir/output/data/subdir
 * auto path2 = resolver.resolvePath("data/subdir");
 *
 * // Absolute input -> /absolute/path/file.txt (unchanged)
 * auto path3 = resolver.resolvePath("/absolute/path/file.txt");
 * ```
 */
class FlexiblePathResolver : public IPathResolver {

private:

    /**
     * @brief Base directory for relative path resolution
     *
     * Default directory that serves as the root for empty and relative
     * path resolution, while absolute paths bypass this entirely.
     */
    std::string defaultDirectory;

public:

    /**
     * @brief Constructor initializing resolver with base directory
     * @param defaultDir Default directory for path resolution operations
     */
    explicit FlexiblePathResolver(const std::string& defaultDir = "export");

    /**
     * @brief Resolves input path with flexible relative path handling
     *
     * Implements adaptive path resolution that preserves relative path structure
     * while providing sensible defaults for empty inputs and absolute path passthrough.
     *
     * ## Resolution Logic
     * - **Empty input**: Returns current_working_directory / defaultDirectory
     * - **Absolute path**: Returns path unchanged
     * - **Relative path**: Returns current_working_directory / defaultDirectory / inputPath
     *
     * @param inputPath Input path string to resolve
     * @return Resolved filesystem path
     * @throws std::filesystem::filesystem_error if current_path() fails
     *
     * @note Unlike DefaultPathResolver, relative paths are appended to the default
     *       directory rather than being ignored, enabling flexible path structures
     *
     * @example
     * ```cpp
     * FlexiblePathResolver resolver("output");
     * auto path1 = resolver.resolvePath("data/file.txt");
     * // Returns: /current/working/dir/output/data/file.txt
     * ```
     */
    fs::path resolvePath(const std::string& inputPath) const override;

};
