#include "DataWriter.h"
#include <iostream>

DataWriter::DataWriter(
    std::shared_ptr<DataFormat> data,
    std::shared_ptr<IFormatter> formatter,
    std::shared_ptr<IOutputTarget> output)
    : m_data(std::move(data)), m_formatter(std::move(formatter)), m_outputTarget(std::move(output))
{
}

bool DataWriter::write()
{
    if (!validate())
    {
        std::cerr << "DataWriter validation failed" << std::endl;
        return false;
    }

    // Format the data
    std::string formattedContent = m_formatter->format(*m_data);

    // Write to output target
    return m_outputTarget->write(formattedContent);
}

void DataWriter::setData(std::shared_ptr<DataFormat> data)
{
    m_data = std::move(data);
}

void DataWriter::setFormatter(std::shared_ptr<IFormatter> formatter)
{
    m_formatter = std::move(formatter);
}

void DataWriter::setOutputTarget(std::shared_ptr<IOutputTarget> output)
{
    m_outputTarget = std::move(output);
}

bool DataWriter::validate() const
{
    if (!m_data)
    {
        std::cerr << "No data provided" << std::endl;
        return false;
    }

    if (!m_data->isValid())
    {
        std::cerr << "Data format is invalid" << std::endl;
        return false;
    }

    if (!m_formatter)
    {
        std::cerr << "No formatter provided" << std::endl;
        return false;
    }

    if (!m_outputTarget)
    {
        std::cerr << "No output target provided" << std::endl;
        return false;
    }

    if (!m_outputTarget->isReady())
    {
        std::cerr << "Output target is not ready" << std::endl;
        return false;
    }

    return true;
}
