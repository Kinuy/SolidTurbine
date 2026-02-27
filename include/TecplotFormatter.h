#ifndef TECPLOTFORMATTER_H
#define TECPLOTFORMATTER_H

#include "IFormatter.h"
#include <sstream>
#include <iomanip>

/**
 * @brief Formatter for Tecplot-style data files
 *
 * Implements IFormatter for the specific format shown in the examples
 */
class TecplotFormatter : public IFormatter
{
public:
    TecplotFormatter() = default;

    std::string format(const DataFormat &data) const override;

private:
    std::string formatVariables(const std::vector<std::string> &vars) const;
    std::string formatZoneHeader(const DataZone &zone) const;
    std::string formatZoneData(const DataZone &zone) const;
};

#endif // TECPLOTFORMATTER_H
