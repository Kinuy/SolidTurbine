#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include "DXFTypes.h"
#include "DXFInterfaces.h"
#include "DXFCore.h"
#include "DXFEntities.h"
#include "BladeInterpolator.h"


// ============================================================
//  DXFDocument
// ============================================================

/**
 * @brief Main document class for creating and managing DXF files
 *
 * Coordinates the entire DXF export process by managing entities, the writer,
 * and the document structure. Follows RAII: the DXF header is written in the
 * constructor and the file is finalized (entities flushed, EOF written) in the
 * destructor — no manual finalization required.
 *
 * Dependency injection via constructor parameters makes the class testable
 * and decoupled from concrete I/O or handle-generation strategies.
 */
class DXFDocument {
private:
    std::vector<std::unique_ptr<IDXFEntity>> entities;
    std::unique_ptr<IDXFWriter>              writer;
    std::unique_ptr<IDXFHandleGenerator>     handleGenerator;
    int                                      handleCounter;

    /** @brief Writes HEADER, TABLES, and BLOCKS sections on construction */
    void writeDocumentStructure();

    /** @brief Writes all accumulated entities to the file (called from destructor) */
    void exportEntities();

public:
    /**
     * @brief Constructs the document and immediately writes the DXF preamble
     * @param w   Writer implementation (must be open, ownership transferred)
     * @param hg  Handle generator (optional, defaults to DXFHandleGenerator(100))
     * @throws std::runtime_error if writer is null or not open
     */
    explicit DXFDocument(
        std::unique_ptr<IDXFWriter>          w,
        std::unique_ptr<IDXFHandleGenerator> hg = std::make_unique<DXFHandleGenerator>());

    /**
     * @brief Destructor — flushes entities, closes ENTITIES section, writes EOF
     */
    ~DXFDocument();

    /** @brief Adds a pre-created entity (ownership transferred) */
    void addEntity(std::unique_ptr<IDXFEntity> entity);

    /** @brief Adds a LINE entity */
    void addLine(const DXFPoint3D& start, const DXFPoint3D& end,
                 const DXFColor& color = DXFColor());

    /** @brief Adds a LWPOLYLINE entity */
    void addPolyLine(const std::vector<DXFPoint3D>& points, bool closed = false,
                     const DXFColor& color = DXFColor());

    /** @brief Adds a TEXT entity */
    void addText(const DXFPoint3D& position, const std::string& text,
                 double height = 1.0, const DXFColor& color = DXFColor());

    /** @brief Adds a POINT entity */
    void addPoint(const DXFPoint3D& position, const DXFColor& color = DXFColor());
};


// ============================================================
//  DXFBlade3D
// ============================================================

/**
 * @brief Creates a 3D DXF representation of a wind turbine blade
 *
 * Uses BladeInterpolator to retrieve blade section geometry, then builds
 * a DXFDocument with one POINT per coordinate and one closed LWPOLYLINE
 * per airfoil section. The DXF file is written immediately on construction.
 */
class DXFBlade3D {
private:
    std::unique_ptr<BladeInterpolator>      bladeInterpolator;
    std::string                             dxfFilePath;
    std::vector<DXFPoint3D>                 dxfPointList;
    std::vector<std::vector<DXFPoint3D>>    dxfPolyLineList;

    /** @brief Populates dxfPointList from all blade section coordinates */
    void fillDxfPointList();

    /** @brief Populates dxfPolyLineList — one polyline per airfoil section */
    void fillDxfPolyLineList();

    /** @brief Calls fill* helpers */
    void createDXFModel();

    /** @brief Constructs a DXFDocument and writes all entities to file */
    void writeDXFFile() const;

public:
    /**
     * @brief Constructs the blade exporter and immediately produces the DXF file
     * @param bladeInterpolator  Source of blade section data (ownership transferred)
     */
    explicit DXFBlade3D(std::unique_ptr<BladeInterpolator> bladeInterpolator);

    ~DXFBlade3D();

    /**
     * @brief Returns a tab-separated text representation of all blade section data
     * @return Multi-line string with name, thickness, radius, chord, and coordinates
     */
    std::string dataToString() const;
};
