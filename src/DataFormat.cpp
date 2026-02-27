#include "DataFormat.h"

DataFormat::DataFormat(const std::string &title)
    : m_title(title)
{
}

void DataFormat::setTitle(const std::string &title)
{
    m_title = title;
}

void DataFormat::setVariables(const std::vector<std::string> &vars)
{
    m_variables = vars;
}

void DataFormat::addZone(const DataZone &zone)
{
    m_zones.push_back(zone);
}

bool DataFormat::isValid() const
{
    if (m_title.empty() || m_variables.empty() || m_zones.empty())
    {
        return false;
    }

    // Validate each zone has correct number of columns
    size_t expectedCols = m_variables.size();
    for (const auto &zone : m_zones)
    {
        for (const auto &row : zone.data)
        {
            if (row.size() != expectedCols)
            {
                return false;
            }
        }
    }

    return true;
}
