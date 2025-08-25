#include "DXFText.h"



DXFText::DXFText(const DXFPoint3D& pos, const std::string& txt, double h, const DXFColor& color)
    : DXFEntityBase(color), position(pos), text(txt), height(h) {
}


void DXFText::writeTo(IDXFWriter& writer, int& handleCounter) const  {
    std::string handle = std::to_string(handleCounter++);
    writeEntityHeader(writer, handle);
    writer.writeGroupCode(100, "AcDbText");
    writer.writeGroupCode(10, position.x);
    writer.writeGroupCode(20, position.y);
    writer.writeGroupCode(30, position.z);
    writer.writeGroupCode(40, height);
    writer.writeGroupCode(1, text);
    writer.writeGroupCode(100, "AcDbText");
}


std::string DXFText::getEntityType() const  { return "TEXT"; }