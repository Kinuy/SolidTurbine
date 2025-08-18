#pragma once


/**
 * @brief Enumeration of available path resolution strategies
 *
 * PathStrategy defines the approach for resolving input paths in export
 * and file management systems. Each strategy has different behavior
 * regarding how relative paths and user input are handled.
 *
 * ## Strategy Characteristics
 * - **DEFAULT**: Restrictive approach, redirects relative paths to default directory
 * - **FLEXIBLE**: Permissive approach, preserves relative path structure under base directory
 *
 * ## Selection Guidelines
 * - Use **DEFAULT** for predictable file organization and centralized output
 * - Use **FLEXIBLE** when users need control over subdirectory structure
 *
 * @see IPathResolver for implementation interface
 * @see DefaultPathResolver, FlexiblePathResolver for concrete implementations
 * @see ExporterFactory for usage in factory pattern
 *
 * @example
 * ```cpp
 * PathStrategy strategy = PathStrategy::FLEXIBLE;
 * auto exporter = ExporterFactory::createExporter(strategy, "/output");
 * ```
 */
enum class PathStrategy{

    /**
     * @brief Conservative path resolution using default directory
     * Ignores relative user input in favor of configured default location
     */
	DEFAULT,

    /**
     * @brief Flexible path resolution preserving user-specified structure
     * Combines relative user input with base directory for organized output
     */
	FLEXIBLE

};
