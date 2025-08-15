#include "LineParser.h"


LineParser::ParsedLine LineParser::parseLine(const std::string& line) {
    ParsedLine result;

    // Trim whitespace
    std::string trimmed = trim(line);

    // Check if empty or comment
    if (trimmed.empty()) {
        result.isEmpty = true;
        return result;
    }
    else {
        result.isEmpty = false;
    }

    if (trimmed[0] == '#' || trimmed[0] == ';') {
        result.isComment = true;
        return result;
    }
    else {
        result.isComment = false;
    }

    // Parse key and all values
    std::istringstream iss(trimmed);
    if (!(iss >> result.key)) {
        throw std::invalid_argument("Invalid line format: " + line);
    }

    // Read all remaining values
    std::string value;
    while (iss >> value) {
        result.values.push_back(value);
    }

    if (result.values.empty()) {
        throw std::invalid_argument("No values found on line: " + line);
    }

    // Set single value for backward compatibility
    result.value = result.values[0];
    result.hasMultipleValues = result.values.size() > 1;

    return result;
}

std::string LineParser::trim(const std::string& str) {
    // First remove inline comments
    size_t comment_pos = str.find('#');
    std::string without_comment = (comment_pos != std::string::npos)
        ? str.substr(0, comment_pos)
        : str;
    // Then trim whitespace from both ends
    size_t start = without_comment.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = without_comment.find_last_not_of(" \t\r\n");
    return without_comment.substr(start, end - start + 1);
}