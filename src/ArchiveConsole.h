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
    bool createArchive(const std::string& archiveName, int argc, char* argv[]);
    bool extractArchive(const std::string& archiveName, const std::string& outputDir = ".");
    bool listArchiveContents(const std::string& archiveName) const;

    /**
     * @brief Add files to an existing archive
     * @param archiveName Path to the archive
     * @param argc Argument count (files start at index 3)
     * @param argv Argument values containing files/directories to add
     * @return true if successful
     */
    bool addToArchive(const std::string& archiveName, int argc, char* argv[]);

    /**
     * @brief Create a self-extracting executable
     * @param outputPath Path for the output executable
     * @param argc Argument count
     * @param argv Argument values containing options and files
     * @return true if successful
     *
     * Options (parsed from argv):
     *   --exec <command>    Command to run after extraction
     *   --args <arguments>  Arguments for the command
     *   --silent            Run command without showing window
     *   --no-wait           Don't wait for command completion
     *   --workdir <dir>     Working directory for command
     *   --stub <path>       Use custom extractor executable
     *   --best              Use best compression
     *   --fastest           Use fastest compression
     */
    bool createSelfExtracting(const std::string& outputPath, int argc, char* argv[]);

    // Configuration setters
    void setCompressionType(CompressionType type) { compressionType = type; }
    void setVerbose(bool verbose) { verboseOutput = verbose; }

private:
    CompressionType compressionType = CompressionType::Normal;
    bool promptOverwrite = true;
    bool verboseOutput = true;
    std::string defaultExtractPath = ".";
    std::string defaultComment;
    ArchiveProgress progress;

    /**
     * @brief Parse command-line options for self-extracting creation
     * @param argc Argument count
     * @param argv Argument values
     * @param config Output auto-exec configuration
     * @param files Output vector of files to include
     * @param stubPath Output custom stub path (if specified)
     * @return Index of first non-option argument, or -1 on error
     */
    int parseSelfExtractOptions(int argc, char* argv[],
                                AutoExecConfig& config,
                                std::vector<std::filesystem::path>& files,
                                std::string& stubPath);
};

#endif // ARCHIVE_CONSOLE_H