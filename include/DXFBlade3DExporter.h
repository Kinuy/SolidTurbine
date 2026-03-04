#pragma once
/**
 * @file DXFBlade3DExporter.h
 * @brief IBlade3DExporter implementation that writes a DXF blade geometry file.
 *
 * Mirrors the structure of TecplotBlade3DExporter — Export() receives the
 * output path and returns true/false, with no side-effects on construction.
 *
 * OCP: registered alongside TecplotBlade3DExporter; main.cpp uses the shared
 * IBlade3DExporter interface and never touches DXFDocument directly.
 */
#include "IBlade3DExporter.h"
#include <string>
#include "DXFDocument.h"
#include "BladeInterpolator.h"
#include <iostream>
#include <stdexcept>
#include <vector>

class BladeInterpolator;

class DXFBlade3DExporter final : public IBlade3DExporter
{
public:
    DXFBlade3DExporter() = default;

    /**
     * @brief Exports blade section geometry to a DXF file.
     *
     * Creates one POINT per blade section coordinate (yellow, ACI 2) and
     * one closed LWPOLYLINE per airfoil section (cyan, ACI 4).
     *
     * @param interpolator  Source of blade section data (non-owning view).
     * @param output_path   Destination .dxf file path (including directory).
     * @return true on success, false if an exception is caught.
     */
    bool Export(BladeInterpolator const& interpolator,
                std::string const& output_path) const override;
};
