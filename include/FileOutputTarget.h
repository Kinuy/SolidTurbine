#ifndef FILEOUTPUTTARGET_H
#define FILEOUTPUTTARGET_H

#include "IOutputTarget.h"
#include <fstream>
#include <filesystem>

/**
 * @brief File-based output target
 *
 * Concrete implementation of IOutputTarget for file writing
 */
class FileOutputTarget : public IOutputTarget
{
public:
    explicit FileOutputTarget(const std::filesystem::path &filePath);
    ~FileOutputTarget() override = default;

    bool write(const std::string &content) override;
    bool isReady() const override;

    const std::filesystem::path &getFilePath() const { return m_filePath; }

private:
    std::filesystem::path m_filePath;
};

#endif // FILEOUTPUTTARGET_H
