#include "DXFPolyLine.h"



DXFPolyLine::DXFPolyLine(
    const std::vector<DXFPoint3D>& pts, 
    bool isClosed, 
    const DXFColor& color)
    : 
    DXFEntityBase(color), 
    points(pts), 
    closed(isClosed) {
}


void DXFPolyLine::writeTo(IDXFWriter& writer, int& handleCounter) const  {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbPolyline");
    writer.writeGroupCode(90, static_cast<int>(points.size()));
    writer.writeGroupCode(70, closed ? 1 : 0);

    for (const auto& point : points) {
        writer.writeGroupCode(10, point.x);
        writer.writeGroupCode(20, point.y);
    }
}


std::string DXFPolyLine::getEntityType() const  { return "LWPOLYLINE"; }