#include "DXFDocument.h"

// ============================================================
//  DXFDocument
// ============================================================

DXFDocument::DXFDocument(std::unique_ptr<IDXFWriter> w, std::unique_ptr<IDXFHandleGenerator> hg)
    : writer(std::move(w)), handleGenerator(std::move(hg)), handleCounter(100)
{
    if (!writer || !writer->isOpen())
    {
        throw std::runtime_error("Invalid writer provided");
    }
    writeDocumentStructure();
}

DXFDocument::~DXFDocument()
{
    if (writer && writer->isOpen())
    {
        exportEntities();
        DXFFormatter::endEntities(*writer);
        DXFFormatter::writeEOF(*writer);
    }
}

void DXFDocument::addEntity(std::unique_ptr<IDXFEntity> entity)
{
    entities.push_back(std::move(entity));
}

void DXFDocument::addLine(const DXFPoint3D &start, const DXFPoint3D &end, const DXFColor &color)
{
    addEntity(DXFEntityFactory::createLine(start, end, color));
}

void DXFDocument::addPolyLine(const std::vector<DXFPoint3D> &points, bool closed, const DXFColor &color)
{
    addEntity(DXFEntityFactory::createPolyLine(points, closed, color));
}

void DXFDocument::addText(const DXFPoint3D &position, const std::string &text,
                          double height, const DXFColor &color)
{
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

// ============================================================
//  DXFBlade3D
// ============================================================

DXFBlade3D::DXFBlade3D(std::unique_ptr<BladeInterpolator> bI)
    : bladeInterpolator(std::move(bI)), dxfFilePath("output/BladeSections3D.dxf")
{
    dxfPointList.clear();
    createDXFModel();
    writeDXFFile();
}

DXFBlade3D::~DXFBlade3D() {}

void DXFBlade3D::createDXFModel()
{
    fillDxfPointList();
    fillDxfPolyLineList();
}

void DXFBlade3D::fillDxfPointList()
{
    const auto &bladeSections = bladeInterpolator->getBladeSections();
    for (size_t s = 0; s < bladeSections.size(); s++)
    {
        auto bladeSectionsCoord = bladeSections.at(s)->airfoilGeometry->getScaledAndRotatedCoordinates();
        for (size_t c = 0; c < bladeSectionsCoord.size(); c++)
        {
            dxfPointList.emplace_back(
                bladeSectionsCoord.at(c).x,
                bladeSectionsCoord.at(c).y,
                bladeSectionsCoord.at(c).z);
        }
    }
}

void DXFBlade3D::fillDxfPolyLineList()
{
    const auto &bladeSections = bladeInterpolator->getBladeSections();
    for (size_t s = 0; s < bladeSections.size(); s++)
    {
        std::vector<DXFPoint3D> airfoilPolyPoints;
        auto bladeSectionsCoord = bladeSections.at(s)->airfoilGeometry->getScaledAndRotatedCoordinates();
        for (size_t c = 0; c < bladeSectionsCoord.size(); c++)
        {
            airfoilPolyPoints.emplace_back(
                bladeSectionsCoord.at(c).x,
                bladeSectionsCoord.at(c).y,
                bladeSectionsCoord.at(c).z);
        }
        dxfPolyLineList.push_back(std::move(airfoilPolyPoints));
    }
}

void DXFBlade3D::writeDXFFile() const
{
    try
    {
        auto fileWriter = std::make_unique<DXFFileWriter>(dxfFilePath);
        DXFDocument document(std::move(fileWriter));

        for (const auto &point : dxfPointList)
        {
            document.addPoint(point, DXFColor(2)); // Yellow — blade geometry points
        }
        for (const auto &polyLine : dxfPolyLineList)
        {
            document.addPolyLine(polyLine, true, DXFColor(4)); // Cyan — airfoil sections
        }

        std::cout << "DXF file '" << dxfFilePath << "' created successfully!\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

std::string DXFBlade3D::dataToString() const
{
    const auto &bladeSections = bladeInterpolator->getBladeSections();
    std::string dataString;
    for (const auto &section : bladeSections)
    {
        dataString += "NAME\t" + section->airfoilName + "\n";
        dataString += "RELTHICK[%]\t" + std::to_string(section->relativeThickness) + "\n";
        dataString += "RADIUS[m]\t" + std::to_string(section->bladeRadius) + "\n";
        dataString += "CHORD[m]\t" + std::to_string(section->chord) + "\n";
        dataString += "#\tX\tY\n";
        for (const auto &coord : section->airfoilGeometry->getCoordinates())
        {
            dataString += section->type + "\t" + std::to_string(coord.x) + "\t" + std::to_string(coord.y) + "\n";
        }
        dataString += "\n";
    }
    return dataString;
}
