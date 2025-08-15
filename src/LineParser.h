#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

// Line parser handles individual line parsing
class LineParser {
private:
    static std::string trim(const std::string& str);

public:
    struct ParsedLine {
        std::string key;
        std::string value;  // For single values
        std::vector<std::string> values;  // For multiple values
        bool isEmpty;
        bool isComment;
        bool hasMultipleValues;
    };

    static ParsedLine parseLine(const std::string& line);


};
