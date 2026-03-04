/**
 * @file DXFBlade3DExporter.cpp
 */
#include "DXFBlade3DExporter.h"


bool DXFBlade3DExporter::Export(BladeInterpolator const& interpolator,
                                std::string const& output_path) const
{
    try
    {
        auto fileWriter = std::make_unique<DXFFileWriter>(output_path);
        DXFDocument document(std::move(fileWriter));

        const auto& sections = interpolator.getBladeSections();

        for (const auto& section : sections)
        {
            const auto coords =
                section->airfoilGeometry->getScaledAndRotatedCoordinates();

            // One POINT per coordinate — yellow (ACI 2)
            for (const auto& c : coords)
                document.addPoint({c.x, c.y, c.z}, DXFColor(2));

            // One closed LWPOLYLINE per airfoil section — cyan (ACI 4)
            std::vector<DXFPoint3D> poly;
            poly.reserve(coords.size());
            for (const auto& c : coords)
                poly.emplace_back(c.x, c.y, c.z);

            document.addPolyLine(poly, /*closed=*/true, DXFColor(4));
        }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DXFBlade3DExporter: " << e.what() << '\n';
        return false;
    }
}
