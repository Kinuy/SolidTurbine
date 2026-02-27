#ifndef DATAWRITER_H
#define DATAWRITER_H

#include "IDataWriter.h"
#include "IFormatter.h"
#include "IOutputTarget.h"
#include "DataFormat.h"
#include <memory>

/**
 * @brief Main data writer class
 *
 * Follows Single Responsibility and Dependency Inversion:
 * - Orchestrates the writing process
 * - Depends on abstractions (IFormatter, IOutputTarget) not concrete types
 */
class DataWriter : public IDataWriter
{
public:
    DataWriter(
        std::shared_ptr<DataFormat> data,
        std::shared_ptr<IFormatter> formatter,
        std::shared_ptr<IOutputTarget> output);

    ~DataWriter() override = default;

    bool write() override;

    // Configuration
    void setData(std::shared_ptr<DataFormat> data);
    void setFormatter(std::shared_ptr<IFormatter> formatter);
    void setOutputTarget(std::shared_ptr<IOutputTarget> output);

private:
    std::shared_ptr<DataFormat> m_data;
    std::shared_ptr<IFormatter> m_formatter;
    std::shared_ptr<IOutputTarget> m_outputTarget;

    bool validate() const;
};

#endif // DATAWRITER_H
