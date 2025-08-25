#include "DXFDocument.h"


DXFDocument::DXFDocument(std::unique_ptr<IDXFWriter> w, std::unique_ptr<IDXFHandleGenerator> hg)
    : writer(std::move(w)), handleGenerator(std::move(hg)), handleCounter(100) {

    if (!writer || !writer->isOpen()) {
        throw std::runtime_error("Invalid writer provided");
    }

    writeDocumentStructure();
}


DXFDocument::~DXFDocument() {
    if (writer && writer->isOpen()) {
        exportEntities();
        DXFFormatter::endEntities(*writer);
        DXFFormatter::writeEOF(*writer);
    }
}


void DXFDocument::addEntity(std::unique_ptr<IDXFEntity> entity) {
    entities.push_back(std::move(entity));
}


void DXFDocument::addLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color) {
    addEntity(DXFEntityFactory::createLine(start, end, color));
}


void DXFDocument::addPolyLine(const std::vector<DXFPoint3D>& points, bool closed, const DXFColor& color) {
    addEntity(DXFEntityFactory::createPolyLine(points, closed, color));
}


void DXFDocument::addText(const DXFPoint3D& position, const std::string& text, double height, const DXFColor& color) {
    addEntity(DXFEntityFactory::createText(position, text, height, color));
}


void DXFDocument::addPoint(const DXFPoint3D& position, const DXFColor& color) {
    addEntity(DXFEntityFactory::createPoint(position, color));
}


void DXFDocument::writeDocumentStructure() {
    DXFFormatter::writeHeader(*writer);
    DXFFormatter::writeTables(*writer);
    DXFFormatter::writeBlocks(*writer);
    DXFFormatter::startEntities(*writer);
}


void DXFDocument::exportEntities() {
    for (const auto& entity : entities) {
        entity->writeTo(*writer, handleCounter);
    }
}