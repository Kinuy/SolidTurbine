#pragma once

#include <string>

/**
 * @brief Interface for generating unique entity handles
 *
 * Abstracts handle generation strategy allowing for different numbering
 * schemes or external handle management systems.
 */
class IDXFHandleGenerator {

public:


    virtual ~IDXFHandleGenerator() = default;

    /**
     * @brief Generates the next unique handle
     * @return String representation of a unique handle
     */
    virtual std::string getNextHandle() = 0;

};
