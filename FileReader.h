#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

#include "IFileReader.h"

// Concrete implementation of a file reader
class FileReader : public IFileReader {

private:

    std::string filename;

public:

    explicit FileReader(const std::string& filename);

    std::vector<std::string> readLines();
};

