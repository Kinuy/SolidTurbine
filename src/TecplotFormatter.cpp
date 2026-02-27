#include "TecplotFormatter.h"

std::string TecplotFormatter::format(const DataFormat &data) const
{
    std::ostringstream oss;

    // Write title
    oss << "TITLE=\"" << data.getTitle() << "\"\n";

    // Write variables
    oss << formatVariables(data.getVariables()) << "\n";

    // Write all zones
    for (const auto &zone : data.getZones())
    {
        oss << formatZoneHeader(zone) << "\n";
        oss << formatZoneData(zone);
    }

    return oss.str();
}

std::string TecplotFormatter::formatVariables(const std::vector<std::string> &vars) const
{
    std::ostringstream oss;
    oss << "VARIABLES=";

    for (size_t i = 0; i < vars.size(); ++i)
    {
        oss << "\"" << vars[i] << "\"";
        if (i < vars.size() - 1)
        {
            oss << " ";
        }
    }

    return oss.str();
}

std::string TecplotFormatter::formatZoneHeader(const DataZone &zone) const
{
    std::ostringstream oss;
    oss << "ZONE I=" << zone.I;

    if (zone.J > 0)
    {
        oss << ", J=" << zone.J;
    }

    if (zone.K > 0)
    {
        oss << ", K=" << zone.K;
    }

    if (zone.dataPacking != "POINT")
    {
        oss << ", DATAPACKING=" << zone.dataPacking;
    }

    if (!zone.title.empty())
    {
        oss << ",T=\"" << zone.title << "\"";
    }

    return oss.str();
}

std::string TecplotFormatter::formatZoneData(const DataZone &zone) const
{
    std::ostringstream oss;

    for (const auto &row : zone.data)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            oss << std::fixed << std::setprecision(1) << row[i];
            if (i < row.size() - 1)
            {
                oss << " ";
            }
        }
        oss << "\n";
    }

    return oss.str();
}
