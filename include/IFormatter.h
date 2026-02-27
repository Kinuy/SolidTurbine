#ifndef IFORMATTER_H
#define IFORMATTER_H

#include "DataFormat.h"
#include <string>

/**
 * @brief Interface for formatting data into specific file formats
 *
 * Following Open/Closed Principle - open for extension (new formats),
 * closed for modification
 */
class IFormatter
{
public:
    virtual ~IFormatter() = default;

    /**
     * @brief Format the data into a string representation
     * @param data The data format to convert
     * @return Formatted string ready to write
     */
    virtual std::string format(const DataFormat &data) const = 0;
};

#endif // IFORMATTER_H