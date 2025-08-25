#pragma once


#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "IDXFEntity.h"
#include "IDXFWriter.h"
#include "IDXFHandleGenerator.h"
#include "DXFFormatter.h"
#include "DXFEntityFactory.h"
#include "DXFHandleGenerator.h"
#include "DXFPoint3D.h"
#include "DXFColor.h"
#include "DXFEntityBase.h"
#include "DXFLine.h"
#include "DXFPolyline.h"
#include "DXFText.h"
#include "DXFPoint.h"


/**
 * @brief Main document class for creating and managing DXF files
 *
 * Coordinates the entire DXF export process by managing entities, writers,
 * and document structure. Follows Single Responsibility Principle by focusing
 * only on document-level operations. Uses dependency injection for flexibility
 * and testability.
 *
 * @note Automatically writes DXF structure in constructor and destructor
 * @note Uses RAII for proper resource management
 * @note Thread-safe for single-threaded usage
 */
class DXFDocument {


private:

    /**
     * @brief Collection of document entities
     */
    std::vector<std::unique_ptr<IDXFEntity>> entities;

    /**
     * @brief Output writer interface
     */
    std::unique_ptr<IDXFWriter> writer;

    /**
     * @brief Handle generation strategy
     */
    std::unique_ptr<IDXFHandleGenerator> handleGenerator;

    /**
     * @brief Current handle counter
     */
    int handleCounter;

    /**
     * @brief Writes DXF document structure (header, tables, blocks)
     *
     * Called automatically in constructor to establish proper DXF file format.
     */
    void writeDocumentStructure();

    /**
     * @brief Exports all entities to the DXF file
     *
     * Called automatically in destructor to write all accumulated entities.
     */
    void exportEntities();

public:

    /**
     * @brief Constructs DXF document with writer and optional handle generator
     * @param w Unique pointer to writer implementation (required)
     * @param hg Unique pointer to handle generator (optional, defaults to HandleGenerator)
     * @throws std::runtime_error if writer is null or not open
     *
     * Automatically writes DXF header, tables, and blocks sections upon construction.
     */
    explicit DXFDocument(std::unique_ptr<IDXFWriter> w, std::unique_ptr<IDXFHandleGenerator> hg = std::make_unique<DXFHandleGenerator>());

    /**
     * @brief Destructor - finalizes DXF file
     *
     * Automatically exports all entities, closes entities section,
     * and writes EOF marker. No manual finalization required.
     */
    ~DXFDocument();

    /**
     * @brief Adds a pre-created entity to the document
     * @param entity Unique pointer to entity (ownership transferred)
     *
     * Allows adding custom entities created outside the document.
     * Useful for complex entities or when using custom entity implementations.
     */
    void addEntity(std::unique_ptr<IDXFEntity> entity);

    /**
     * @brief Adds a line to the document
     * @param start Line starting point
     * @param end Line ending point
     * @param color Line color (default: white/black)
     */
    void addLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color = DXFColor());

    /**
     * @brief Adds a polyline to the document
     * @param points Vector of polyline vertices
     * @param closed Whether to close the polyline
     * @param color Polyline color (default: white/black)
     */
    void addPolyLine(const std::vector<DXFPoint3D>& points, bool closed = false, const DXFColor& color = DXFColor());

    /**
     * @brief Adds text to the document
     * @param position Text insertion point
     * @param text Text content
     * @param height Text height in drawing units (default: 1.0)
     * @param color Text color (default: white/black)
     */
    void addText(const DXFPoint3D& position, const std::string& text, double height = 1.0, const DXFColor& color = DXFColor());

    /**
     * @brief Adds a point to the document
     * @param position Point location
     * @param color Point color (default: white/black)
     */
    void addPoint(const DXFPoint3D& position, const DXFColor& color = DXFColor());


};