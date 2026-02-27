#include "FileOutputTarget.h"
#include <iostream>

FileOutputTarget::FileOutputTarget(const std::filesystem::path &filePath)
    : m_filePath(filePath)
{
}

bool FileOutputTarget::write(const std::string &content)
{
    try
    {
        // Create parent directories if they don't exist
        if (m_filePath.has_parent_path())
        {
            std::filesystem::create_directories(m_filePath.parent_path());
        }

        std::ofstream outFile(m_filePath);
        if (!outFile)
        {
            std::cerr << "Failed to open file: " << m_filePath << std::endl;
            return false;
        }

        outFile << content;

        if (!outFile)
        {
            std::cerr << "Failed to write to file: " << m_filePath << std::endl;
            return false;
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception writing file: " << e.what() << std::endl;
        return false;
    }
}

bool FileOutputTarget::isReady() const
{
    // Check if we can potentially write to this location
    if (m_filePath.empty())
    {
        return false;
    }

    // If file exists, check if it's writable
    if (std::filesystem::exists(m_filePath))
    {
        auto perms = std::filesystem::status(m_filePath).permissions();
        return (perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
    }

    // If file doesn't exist, check if parent directory exists and is writable
    auto parentPath = m_filePath.parent_path();
    if (parentPath.empty())
    {
        parentPath = ".";
    }

    if (!std::filesystem::exists(parentPath))
    {
        // We can create it, so it's ready
        return true;
    }

    auto perms = std::filesystem::status(parentPath).permissions();
    return (perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
}
