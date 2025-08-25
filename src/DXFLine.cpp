#include "DXFLine.h"



DXFLine::DXFLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color)
    : DXFEntityBase(color), startPoint(start), endPoint(end) {
}


void DXFLine::writeTo(IDXFWriter& writer, int& handleCounter) const  {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbLine");
    writer.writeGroupCode(10, startPoint.x);
    writer.writeGroupCode(20, startPoint.y);
    writer.writeGroupCode(30, startPoint.z);
    writer.writeGroupCode(11, endPoint.x);
    writer.writeGroupCode(21, endPoint.y);
    writer.writeGroupCode(31, endPoint.z);
}


std::string DXFLine::getEntityType() const  { return "LINE"; }