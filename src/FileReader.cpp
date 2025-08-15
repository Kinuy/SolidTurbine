#include "FileReader.h"

FileReader::FileReader(const std::string& filename): filename(filename) 
{
}

// Reads lines from the file and returns them as a vector of strings
std::vector<std::string> FileReader::readLines(){
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}
