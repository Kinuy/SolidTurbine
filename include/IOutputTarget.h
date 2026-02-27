#ifndef IOUTPUTTARGET_H
#define IOUTPUTTARGET_H

#include <string>

/**
 * @brief Interface for output targets (files, streams, etc.)
 *
 * Following Dependency Inversion Principle - high-level modules
 * depend on this abstraction, not concrete implementations
 */
class IOutputTarget
{
public:
    virtual ~IOutputTarget() = default;

    /**
     * @brief Write content to the target
     * @param content The string content to write
     * @return true if write was successful
     */
    virtual bool write(const std::string &content) = 0;

    /**
     * @brief Check if the target is ready for writing
     * @return true if target is valid and ready
     */
    virtual bool isReady() const = 0;
};

#endif // IOUTPUTTARGET_H