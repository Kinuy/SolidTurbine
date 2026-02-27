#pragma once
/**
 * @file IBlade3DExporter.h
 * @brief Abstract interface for 3D blade geometry export.
 *
 * OCP: new formats (DXF, VTK, …) are added by implementing this interface.
 * DIP: main.cpp depends on this abstraction only.
 */
#include <memory>
#include <string>

class BladeInterpolator;

class IBlade3DExporter
{
public:
    virtual ~IBlade3DExporter() = default;

    /**
     * @brief Export 3D blade section geometry.
     * @param interpolator  Source of blade section data (non-owning view).
     * @param output_path   Destination file path.
     * @return true on success.
     */
    virtual bool Export(BladeInterpolator const &interpolator,
                        std::string const &output_path) const = 0;
};
