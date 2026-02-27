#include "DXFEntities.h"


// ============================================================
//  DXFEntityBase
// ============================================================

DXFEntityBase::DXFEntityBase(const DXFColor& c) : color(c) {}

void DXFEntityBase::writeEntityHeader(IDXFWriter& writer, const std::string& handle) const {
    DXFFormatter::writeEntityHeader(writer, getEntityType(), handle, color);
}


// ============================================================
//  DXFLine
// ============================================================

DXFLine::DXFLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color)
    : DXFEntityBase(color), startPoint(start), endPoint(end) {}

void DXFLine::writeTo(IDXFWriter& writer, int& handleCounter) const {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbLine");
    writer.writeGroupCode(10,  startPoint.x);
    writer.writeGroupCode(20,  startPoint.y);
    writer.writeGroupCode(30,  startPoint.z);
    writer.writeGroupCode(11,  endPoint.x);
    writer.writeGroupCode(21,  endPoint.y);
    writer.writeGroupCode(31,  endPoint.z);
}

std::string DXFLine::getEntityType() const { return "LINE"; }


// ============================================================
//  DXFPoint
// ============================================================

DXFPoint::DXFPoint(const DXFPoint3D& pos, const DXFColor& color)
    : DXFEntityBase(color), position(pos) {}

void DXFPoint::writeTo(IDXFWriter& writer, int& handleCounter) const {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbPoint");
    writer.writeGroupCode(10,  position.x);
    writer.writeGroupCode(20,  position.y);
    writer.writeGroupCode(30,  position.z);
}

std::string DXFPoint::getEntityType() const { return "POINT"; }


// ============================================================
//  DXFPolyLine
// ============================================================

DXFPolyLine::DXFPolyLine(const std::vector<DXFPoint3D>& pts, bool isClosed, const DXFColor& color)
    : DXFEntityBase(color), points(pts), closed(isClosed) {}

void DXFPolyLine::writeTo(IDXFWriter& writer, int& handleCounter) const {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbPolyline");
    writer.writeGroupCode(90,  static_cast<int>(points.size()));
    writer.writeGroupCode(70,  closed ? 1 : 0);

    for (const auto& point : points) {
        writer.writeGroupCode(10, point.x);
        writer.writeGroupCode(20, point.y);
        writer.writeGroupCode(30, point.z);
    }
}

std::string DXFPolyLine::getEntityType() const { return "LWPOLYLINE"; }


// ============================================================
//  DXFText
// ============================================================

DXFText::DXFText(const DXFPoint3D& pos, const std::string& txt, double h, const DXFColor& color)
    : DXFEntityBase(color), position(pos), text(txt), height(h) {}

void DXFText::writeTo(IDXFWriter& writer, int& handleCounter) const {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbText");
    writer.writeGroupCode(10,  position.x);
    writer.writeGroupCode(20,  position.y);
    writer.writeGroupCode(30,  position.z);
    writer.writeGroupCode(40,  height);
    writer.writeGroupCode(1,   text);
    writer.writeGroupCode(100, "AcDbText");   // required duplicate per DXF spec
}

std::string DXFText::getEntityType() const { return "TEXT"; }


// ============================================================
//  DXFEntityFactory
// ============================================================

std::unique_ptr<IDXFEntity> DXFEntityFactory::createLine(
    const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color)
{
    return std::make_unique<DXFLine>(start, end, color);
}

std::unique_ptr<IDXFEntity> DXFEntityFactory::createPolyLine(
    const std::vector<DXFPoint3D>& points, bool closed, const DXFColor& color)
{
    return std::make_unique<DXFPolyLine>(points, closed, color);
}

std::unique_ptr<IDXFEntity> DXFEntityFactory::createText(
    const DXFPoint3D& position, const std::string& text, double height, const DXFColor& color)
{
    return std::make_unique<DXFText>(position, text, height, color);
}

std::unique_ptr<IDXFEntity> DXFEntityFactory::createPoint(
    const DXFPoint3D& position, const DXFColor& color)
{
    return std::make_unique<DXFPoint>(position, color);
}

std::unique_ptr<IDXFEntity> DXFEntityFactory::createRectangle(
    const DXFPoint3D& corner1, const DXFPoint3D& corner2, const DXFColor& color)
{
    std::vector<DXFPoint3D> pts = {
        { corner1.x, corner1.y, corner1.z },
        { corner2.x, corner1.y, corner1.z },
        { corner2.x, corner2.y, corner1.z },
        { corner1.x, corner2.y, corner1.z }
    };
    return std::make_unique<DXFPolyLine>(pts, true, color);
}
