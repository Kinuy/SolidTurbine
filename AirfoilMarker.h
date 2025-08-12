#pragma once

#include <string>

// Responsible for storing airfoil markers in a structured way
struct AirfoilMarker {
    std::string type;    // TEE, TESSMAX, TESS, etc.
    int index;           // Index position in coordinate array

    AirfoilMarker(const std::string& type, int index) : type(type), index(index) {}
};