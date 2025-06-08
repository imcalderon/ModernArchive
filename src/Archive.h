/**
 * @file Archive.h
 * @brief Modern reimplementation of the Sonic Foundry archive format
 * - Uses standard ZLIB compression instead of custom algorithms
 * - Supports cross-platform operation
 * - Implements modern C++ practices and RAII
 * - Provides comprehensive error handling
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include "CompressionTypes.h"
#include "ArchiveFormat.h"

/**
 * @struct ArchiveEntry
 * @brief Represents a single file entry in the archive
 */
struct ArchiveEntry {
    std::string name;           ///< File name (platform-independent format)
    uint64_t compressedSize;    ///< Size of the compressed data
    uint64_t originalSize;      ///< Original file size
    time_t timestamp;           ///< File modification time
};

/**
 * @class Archive
 * @brief Main archive management class
 */
class Archive {
public:
    /**
     * @brief Constructs an Archive object
     * @param archiveName Path to the archive file
     */
    explicit Archive(const std::string& archiveName);

    /**
     * @brief Creates a new archive with the specified files
     * @param files List of files to include
     * @param compression Compression level to use
     */
    void create(const std::vector<std::filesystem::path>& files, 
                CompressionType compression = CompressionType::Normal);

    /**
     * @brief Adds files to an existing archive
     * @param files List of files to add
     * @param compression Compression level to use
     */
    void add(const std::vector<std::filesystem::path>& files,
             CompressionType compression = CompressionType::Normal);

    /**
     * @brief Extracts the archive contents
     * @param outputDir Directory to extract files to
     */
    void extract(const std::string& outputDir);

    /**
     * @brief Gets the list of files in the archive
     * @return Vector of file entries
     */
    std::vector<ArchiveEntry> getFileList() const { return entries; }

private:
    std::string archiveName;
    std::vector<ArchiveEntry> entries;

    /**
     * @brief Adds a single file to the archive
     * @param file Path to the file
     * @param archivePath Path to store in the archive
     * @param archive Output stream for the archive
     * @param compression Compression level to use
     */
    void addFileToArchive(const std::filesystem::path& file, 
                         const std::string& archivePath,
                         std::ofstream& archive, 
                         CompressionType compression);
                         
    /**
     * @brief Compresses data using ZLIB
     * @param input Input data
     * @param compression Compression level
     * @return Compressed data
     */
    std::vector<char> compressData(const std::vector<char>& input, 
                                  CompressionType compression);
};