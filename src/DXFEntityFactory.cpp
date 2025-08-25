#include "DXFEntityFactory.h"



 std::unique_ptr<IDXFEntity> DXFEntityFactory::createLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color = DXFColor()) {
    return std::make_unique<DXFLine>(start, end, color);
}


 std::unique_ptr<IDXFEntity> DXFEntityFactory::createPolyLine(const std::vector<DXFPoint3D>& points, bool closed = false, const DXFColor& color = DXFColor()) {
    return std::make_unique<DXFPolyLine>(points, closed, color);
}


 std::unique_ptr<IDXFEntity> DXFEntityFactory::createText(const DXFPoint3D& position, const std::string& text, double height = 1.0, const DXFColor& color = DXFColor()) {
    return std::make_unique<DXFText>(position, text, height, color);
}


 std::unique_ptr<IDXFEntity> DXFEntityFactory::createPoint(const DXFPoint3D& position, const DXFColor& color = DXFColor()) {
    return std::make_unique<DXFPoint>(position, color);
}


 std::unique_ptr<IDXFEntity> DXFEntityFactory::createRectangle(const DXFPoint3D& corner1, const DXFPoint3D& corner2, const DXFColor& color = DXFColor()) {
    std::vector<DXFPoint3D> points = {
        DXFPoint3D(corner1.x, corner1.y, corner1.z),
        DXFPoint3D(corner2.x, corner1.y, corner1.z),
        DXFPoint3D(corner2.x, corner2.y, corner1.z),
        DXFPoint3D(corner1.x, corner2.y, corner1.z)
    };
    return std::make_unique<DXFPolyLine>(points, true, color);
}