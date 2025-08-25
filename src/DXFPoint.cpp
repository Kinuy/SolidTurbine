#include "DXFPoint.h"



DXFPoint::DXFPoint(const DXFPoint3D& pos, const DXFColor& color)
    : DXFEntityBase(color), position(pos) {
}


void DXFPoint::writeTo(IDXFWriter& writer, int& handleCounter) const  {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbPoint");
    writer.writeGroupCode(10, position.x);
    writer.writeGroupCode(20, position.y);
    writer.writeGroupCode(30, position.z);
}


std::string DXFPoint::getEntityType() const  { return "POINT"; }