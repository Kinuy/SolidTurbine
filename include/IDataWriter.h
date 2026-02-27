#ifndef IDATAWRITER_H
#define IDATAWRITER_H

#include <string>
#include <memory>

/**
 * @brief Abstract interface for data writers
 *
 * Following Interface Segregation Principle - minimal interface
 * for writing data to output streams
 */
class IDataWriter
{
public:
    virtual ~IDataWriter() = default;

    /**
     * @brief Write data to the configured output
     * @return true if write was successful, false otherwise
     */
    virtual bool write() = 0;
};

#endif // IDATAWRITER_H
