#include "DXFFileWriter.h"



DXFFileWriter::DXFFileWriter(const std::string& filename) {
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
}


DXFFileWriter::~DXFFileWriter() {
    if (file.is_open()) {
        file.close();
    }
}


void DXFFileWriter::writeGroupCode(int code, const std::string& value)  {
    file << code << "\n" << value << "\n";
}


void DXFFileWriter::writeGroupCode(int code, double value)  {
    file << code << "\n" << value << "\n";
}


void DXFFileWriter::writeGroupCode(int code, int value)  {
    file << code << "\n" << value << "\n";
}


bool DXFFileWriter::isOpen() const  {
    return file.is_open();
}