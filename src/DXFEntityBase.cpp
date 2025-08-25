#include "DXFEntityBase.h"


DXFEntityBase::DXFEntityBase(const DXFColor& c) : color(c) {}


void DXFEntityBase::writeEntityHeader(IDXFWriter& writer, const std::string& handle) const {
    DXFFormatter::writeEntityHeader(writer, getEntityType(), handle, color);
}
