#include "DXFFormatter.h"


 void DXFFormatter::writeHeader(IDXFWriter& writer) {
    writer.writeGroupCode(0, "SECTION");
    writer.writeGroupCode(2, "HEADER");
    writer.writeGroupCode(9, "$ACADVER");
    writer.writeGroupCode(1, "AC1015");
    writer.writeGroupCode(9, "$INSBASE");
    writer.writeGroupCode(10, 0.0);
    writer.writeGroupCode(20, 0.0);
    writer.writeGroupCode(30, 0.0);
    writer.writeGroupCode(9, "$EXTMIN");
    writer.writeGroupCode(10, 0.0);
    writer.writeGroupCode(20, 0.0);
    writer.writeGroupCode(30, 0.0);
    writer.writeGroupCode(9, "$EXTMAX");
    writer.writeGroupCode(10, 100.0);
    writer.writeGroupCode(20, 100.0);
    writer.writeGroupCode(30, 0.0);
    writer.writeGroupCode(0, "ENDSEC");
}


 void DXFFormatter::writeTables(IDXFWriter& writer) {
    writer.writeGroupCode(0, "SECTION");
    writer.writeGroupCode(2, "TABLES");
    writer.writeGroupCode(0, "TABLE");
    writer.writeGroupCode(2, "LAYER");
    writer.writeGroupCode(5, "2");
    writer.writeGroupCode(100, "AcDbSymbolTable");
    writer.writeGroupCode(70, 1);
    writer.writeGroupCode(0, "LAYER");
    writer.writeGroupCode(5, "10");
    writer.writeGroupCode(100, "AcDbSymbolTableRecord");
    writer.writeGroupCode(100, "AcDbLayerTableRecord");
    writer.writeGroupCode(2, "0");
    writer.writeGroupCode(70, 0);
    writer.writeGroupCode(62, 7);
    writer.writeGroupCode(6, "CONTINUOUS");
    writer.writeGroupCode(0, "ENDTAB");
    writer.writeGroupCode(0, "ENDSEC");
}


 void DXFFormatter::writeBlocks(IDXFWriter& writer) {
    writer.writeGroupCode(0, "SECTION");
    writer.writeGroupCode(2, "BLOCKS");
    writer.writeGroupCode(0, "ENDSEC");
}


 void DXFFormatter::startEntities(IDXFWriter& writer) {
    writer.writeGroupCode(0, "SECTION");
    writer.writeGroupCode(2, "ENTITIES");
}


 void DXFFormatter::endEntities(IDXFWriter& writer) {
    writer.writeGroupCode(0, "ENDSEC");
}


 void DXFFormatter::writeEOF(IDXFWriter& writer) {
    writer.writeGroupCode(0, "EOF");
}


 void DXFFormatter::writeEntityHeader(IDXFWriter& writer, const std::string& entityType,
    const std::string& handle, const DXFColor& color) {
    writer.writeGroupCode(0, entityType);
    writer.writeGroupCode(5, handle);
    writer.writeGroupCode(100, "AcDbEntity");
    writer.writeGroupCode(8, "0");
    writer.writeGroupCode(62, color.colorNumber);
}