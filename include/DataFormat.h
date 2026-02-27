#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <string>
#include <vector>

/**
 * @brief Represents a data zone with its configuration
 *
 * Single Responsibility: Encapsulates zone metadata
 */
struct DataZone
{
    std::string title;
    int I = 0; // I dimension
    int J = 0; // J dimension (optional)
    int K = 0; // K dimension (optional)
    std::string dataPacking = "POINT";
    std::vector<std::vector<double>> data;

    DataZone() = default;
    DataZone(const std::string &t, int i, int j = 0, int k = 0)
        : title(t), I(i), J(j), K(k) {}
};

/**
 * @brief Represents the complete data format structure
 *
 * Single Responsibility: Holds all metadata and data for a file
 */
class DataFormat
{
public:
    DataFormat() = default;
    explicit DataFormat(const std::string &title);

    // Setters
    void setTitle(const std::string &title);
    void setVariables(const std::vector<std::string> &vars);
    void addZone(const DataZone &zone);

    // Getters
    const std::string &getTitle() const { return m_title; }
    const std::vector<std::string> &getVariables() const { return m_variables; }
    const std::vector<DataZone> &getZones() const { return m_zones; }

    // Utility
    size_t getVariableCount() const { return m_variables.size(); }
    bool isValid() const;

private:
    std::string m_title;
    std::vector<std::string> m_variables;
    std::vector<DataZone> m_zones;
};

#endif // DATAFORMAT_H
