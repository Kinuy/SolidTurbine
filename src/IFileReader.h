#pragma once
#include <vector>
#include <string>


// Interface for file reader that handles reading lines from a file.
class IFileReader {
public:
    virtual ~IFileReader() = default;
    virtual std::vector<std::string> readLines() = 0;
};
