/**
 * @file Archive.h
 * @brief Modern reimplementation of the Sonic Foundry archive format with self-extracting support
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include "CompressionTypes.h"
#include "ArchiveFormat.h"

struct ArchiveEntry {
    std::string name;
    uint64_t compressedSize;
    uint64_t originalSize;
    time_t timestamp;
};

/**
 * @brief Configuration for auto-execution after extraction
 */
struct AutoExecConfig {
    std::string command;        ///< Command to execute (e.g., "msiexec")
    std::string arguments;      ///< Command arguments (e.g., "/i installer.msi /quiet")
    bool silent = false;        ///< Run command without showing window
    bool waitForCompletion = true; ///< Wait for command to complete before exiting
    std::string workingDir;     ///< Working directory (empty = extraction directory)
};

class Archive {
public:
    explicit Archive(const std::string& archiveName);

    void create(const std::vector<std::filesystem::path>& files, 
                CompressionType compression = CompressionType::Normal);

    void add(const std::vector<std::filesystem::path>& files,
             CompressionType compression = CompressionType::Normal);

    void extract(const std::string& outputDir);

    /**
     * @brief Creates a self-extracting executable
     * @param files List of files to include
     * @param outputPath Path for the self-extracting executable (e.g., "installer.exe")
     * @param compression Compression level to use
     * @param autoExec Auto-execution configuration (optional)
     * @param stubPath Path to the extractor stub executable (optional, will build if not provided)
     */
    void createSelfExtracting(const std::vector<std::filesystem::path>& files,
                             const std::string& outputPath,
                             CompressionType compression = CompressionType::Normal,
                             const AutoExecConfig& autoExec = {},
                             const std::string& stubPath = "");

    std::vector<ArchiveEntry> getFileList() const { return entries; }

private:
    std::string archiveName;
    std::vector<ArchiveEntry> entries;

    void addFileToArchive(const std::filesystem::path& file, 
                         const std::string& archivePath,
                         std::ofstream& archive, 
                         CompressionType compression);
                         
    std::vector<char> compressData(const std::vector<char>& input, 
                                  CompressionType compression);

    /**
     * @brief Builds the extractor stub executable
     * @param outputPath Where to save the compiled stub
     * @return true if successful
     */
    bool buildExtractorStub(const std::string& outputPath);

    /**
     * @brief Combines stub executable with archive data and command config
     * @param stubPath Path to the stub executable
     * @param archiveData The archive data to embed
     * @param outputPath Path for the final self-extracting executable
     * @param autoExec Auto-execution configuration
     */
    bool combineStubWithArchive(const std::string& stubPath,
                               const std::vector<char>& archiveData,
                               const std::string& outputPath,
                               const AutoExecConfig& autoExec);

    /**
     * @brief Helper method to add file to a stream instead of file
     */
    void addFileToArchiveStream(const std::filesystem::path& file, 
                               const std::string& archivePath,
                               std::ostringstream& archive, 
                               CompressionType compression);
};