#ifndef ARCHIVE_CONSOLE_H
#define ARCHIVE_CONSOLE_H

#include "Archive.h"
#include "ArchiveProgress.h"
#include "CompressionTypes.h"
#include "CrossPlatform.h"
#include <vector>
#include <memory>
#include <map>
#include <string>

class Archive;

class ArchiveConsole {
public:
    ArchiveConsole() = default;
    ~ArchiveConsole() = default;

    void printUsage() const;
    bool createArchive(const std::string& archiveName);
    bool extractArchive(const std::string& archiveName, const std::string& outputDir = ".");
    bool listArchiveContents(const std::string& archiveName) const;

private:
    CompressionType compressionType = CompressionType::Deflate;
    CompressionLevel compressionLevel = CompressionLevel::Normal;
    bool promptOverwrite = true;
    bool verboseOutput = true;
    std::string defaultExtractPath = ".";
    std::string defaultComment;
    ArchiveProgress progress;
};

#endif // ARCHIVE_CONSOLE_H