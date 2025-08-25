#pragma once

#include <string>
#include "IDXFHandleGenerator.h"

/**
 * @brief Sequential handle generator for DXF entities
 *
 * Generates unique sequential handles starting from a configurable number.
 * Follows Single Responsibility Principle by handling only handle generation.
 * Thread-safe for single-threaded usage.
 */
class DXFHandleGenerator : public IDXFHandleGenerator {


private:

    /**
     * @brief Current handle counter
     */
    int currentHandle;

public:

    /**
     * @brief Constructs handle generator with starting value
     * @param startHandle Starting handle number (default: 100)
     */
    explicit DXFHandleGenerator(int startHandle = 100);

    /**
     * @brief Generates next sequential handle
     * @return String representation of next handle number
     */
    std::string getNextHandle() override;

};