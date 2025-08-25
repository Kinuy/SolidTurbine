#pragma once

#include <string>
#include <vector>
#include "DXFEntityBase.h"
#include "DXFPoint3D.h"
#include "DXFColor.h"
#include "IDXFWriter.h"



/**
 * @brief DXF TEXT entity implementation
 *
 * Represents single-line text with position, content, and height.
 * For multi-line text, consider implementing MTEXT entity separately.
 */
class DXFText : public DXFEntityBase {


private:

    /**
     * @brief Text insertion point
     */
    DXFPoint3D position;

    /**
     * @brief Text content string
     */
    std::string text;

    /**
     * @brief Text height (default: 1.0)
     *
     * Represents the height of the text in drawing units.
	 */
    double height;

public:

    /**
     * @brief Constructs a text entity
     * @param pos Text insertion point (typically lower-left corner)
     * @param txt Text content string
     * @param h Text height in drawing units (default: 1.0)
     * @param color Text color (default: white/black)
     */
    DXFText(const DXFPoint3D& pos, const std::string& txt, double h = 1.0, const DXFColor& color = DXFColor());

    /**
     * @brief Writes text entity to DXF format
     * @param writer Output writer interface
     * @param handleCounter Reference to handle counter for unique entity ID
     */
    void writeTo(IDXFWriter& writer, int& handleCounter) const override;

    /**
     * @brief Returns DXF entity type identifier
     * @return "TEXT" as required by DXF specification
     */
    std::string getEntityType() const override;

};
