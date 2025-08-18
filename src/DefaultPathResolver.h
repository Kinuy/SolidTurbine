#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

#include "IPathResolver.h"


/**
 * @brief Simple path resolver implementation that prioritizes a default directory
 *
 * DefaultPathResolver is a concrete implementation of the IPathResolver interface
 * that provides a predictable and conservative approach to path resolution. This
 * resolver is designed for scenarios where output should be concentrated in a
 * specific directory location, regardless of user input variations.
 *
 * ## Resolution Strategy
 *
 * The DefaultPathResolver follows a straightforward strategy:
 * - **Empty paths**: Resolved to `current_working_directory / defaultDirectory`
 * - **Absolute paths**: Preserved unchanged (user has explicit control)
 * - **Relative paths**: Redirected to `current_working_directory / defaultDirectory`
 *
 * This approach ensures that all output (except explicit absolute paths) ends up
 * in the configured default directory, providing predictable file organization
 * and preventing scattered output across the filesystem.
 *
 * ## Use Cases
 *
 * This resolver is particularly useful for:
 * - Applications that need centralized output management
 * - Systems where users shouldn't influence output location via relative paths
 * - Scenarios requiring simple, predictable path behavior
 * - Default configurations where safety and predictability are prioritized
 *
 * ## Comparison with Other Resolvers
 *
 * Unlike flexible path resolvers that might combine user input with base directories,
 * DefaultPathResolver takes a more restrictive approach by ignoring relative user
 * input in favor of the configured default location.
 *
 * @see IPathResolver for the base interface definition
 * @see FlexiblePathResolver for a more permissive alternative
 *
 * @note This resolver does not validate the existence or accessibility of paths
 *       during resolution - it only performs logical path construction
 *
 * @example
 * ```cpp
 * DefaultPathResolver resolver("/output/data");
 *
 * // Empty input -> /current/working/dir/output/data
 * auto path1 = resolver.resolvePath("");
 *
 * // Absolute input -> /absolute/path/file.txt (unchanged)
 * auto path2 = resolver.resolvePath("/absolute/path/file.txt");
 *
 * // Relative input -> /current/working/dir/output/data (input ignored)
 * auto path3 = resolver.resolvePath("some/relative/path");
 * ```
 *
 * @thread_safety This class is thread-safe for read operations after construction.
 *                The defaultDirectory is immutable after initialization.
 */
class DefaultPathResolver : public IPathResolver {

private:

    std::string defaultDirectory;

public:

    /**
     * @brief Constructor that initializes the DefaultPathResolver with a base directory
     *
     * Creates a DefaultPathResolver instance configured with the specified default directory.
     * This directory will be used as the base for all path resolution operations when
     * input paths are empty or relative.
     *
     * @param defaultDir The default directory path to use for path resolution.
     *                   Can be either an absolute or relative path string.
     *                   If relative, it will be resolved relative to the current
     *                   working directory when resolvePath() is called.
     *
     * @note The defaultDir parameter is stored as-is and not validated during construction.
     *       Path validation occurs during resolvePath() calls.
     */
    explicit DefaultPathResolver(const std::string& defaultDir = "export");

    /**
     * @brief Resolves an input path according to the default path resolution strategy
     *
     * Implements a simple path resolution strategy that prioritizes the default directory
     * for most cases. The resolution logic follows these rules:
     *
     * 1. Empty input: Returns current_working_directory / defaultDirectory
     * 2. Absolute input path: Returns the absolute path unchanged
     * 3. Relative input path: Returns current_working_directory / defaultDirectory
     *    (Note: The relative input path itself is ignored in favor of the default)
     *
     * This strategy ensures predictable output locations by consistently using the
     * default directory, except when an explicit absolute path is provided.
     *
     * @param inputPath The input path string to resolve. Can be:
     *                  - Empty string (uses default directory)
     *                  - Absolute path (returned as-is)
     *                  - Relative path (default directory is used instead)
     *
     * @return A filesystem path object representing the resolved absolute path
     *
     * @throws std::filesystem::filesystem_error if current_path() fails to determine
     *         the current working directory
     *
     * @note For relative input paths, this resolver does NOT append the input to the
     *       default directory - it uses only the default directory. This behavior
     *       differs from flexible path resolvers that might combine relative inputs
     *       with the base directory.
     *
     * @see fs::current_path() for current working directory determination
     * @see fs::path::is_absolute() for absolute path detection
     */
    fs::path resolvePath(const std::string& inputPath) const override;

};

