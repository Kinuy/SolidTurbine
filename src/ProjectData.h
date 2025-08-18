#pragma once

#include <string>

/**
 * @brief Data structure for project metadata and identification
 *
 * ProjectData holds essential project information including identification,
 * versioning, and personnel details. Provides default placeholder values
 * for all fields to ensure valid initialization in all scenarios.
 *
 * ## Default Behavior
 * - **Placeholder Values**: All defaults start with "X" prefix for easy identification
 * - **Complete Initialization**: No uninitialized fields, safe for immediate use
 * - **Flexible Construction**: Can specify any subset of parameters
 *
 * ## Common Use Cases
 * - Configuration file headers and metadata
 * - Report generation and document templates
 * - Project tracking and version control
 * - Engineering documentation systems
 *
 * @note Default values are intended as obvious placeholders that should
 *       be replaced with actual project data in production use
 *
 * @example
 * ```cpp
 * // Full specification
 * ProjectData project("PROJ001", "Turbine Analysis", "v2.1",
 *                    "2025-08-18", "ACME Corp", "J. Smith");
 *
 * // Partial specification (remaining fields use defaults)
 * ProjectData quickProject("PROJ002", "Quick Study");
 *
 * // Default construction (all placeholder values)
 * ProjectData defaultProject;
 * ```
 */
struct ProjectData
{
	/**
	 * @brief Unique project identifier or code
	 */
	std::string projectId;

	/**
	 * @brief Human-readable project name or title
	 */
	std::string projectName;

	/**
	 * @brief Version or revision identifier
	 */
	std::string revision;

	/**
	 * @brief Project date (creation, modification, or analysis date)
	 */
	std::string date;

	/**
	 * @brief Customer or client organization name
	 */
	std::string customer;

	/**
	 * @brief Responsible engineer or analyst name
	 */
	std::string engineer;


	/**
	 * @brief Constructor with default placeholder values for all fields
	 *
	 * @param projectId Project identifier (default: "XId")
	 * @param projectName Project name (default: "Xname")
	 * @param revision Version identifier (default: "Xrevision")
	 * @param date Project date (default: "Xdate")
	 * @param customer Customer name (default: "Xcoustomer")
	 * @param engineer Engineer name (default: "Xengineer")
	 */
	ProjectData(
		std::string projectId = "XId", 
		std::string projectName = "Xname", 
		std::string revision = "Xrevision", 
		std::string date = "Xdate", 
		std::string customer = "Xcoustomer", 
		std::string engineer = "Xengineer") 
		:
		projectId(projectId),
		projectName(projectName),
		revision(revision), 
		date(date), 
		customer(customer), 
		engineer(engineer) {
	}

};