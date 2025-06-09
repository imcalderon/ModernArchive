// filepath: e:\proj\ModernArchive\src\Archive.cpp

#include "Archive.h"
#include "ArchiveFormat.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <zlib.h>
#include <ctime>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
#include <cstring>

namespace fs = std::filesystem;

// Helper function to make paths relative and convert to archive format
std::string makeArchivePath(const std::filesystem::path& file, const std::filesystem::path& basePath = {}) {
    std::string result;
    if (basePath.empty()) {
        result = file.filename().string();
    } else {
        result = std::filesystem::relative(file, basePath).string();
    }
    // Convert to forward slashes for cross-platform compatibility
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

Archive::Archive(const std::string& archName) : archiveName(archName) {
    if (fs::exists(archName)) {
        // Try to read existing archive
        std::ifstream file(archName, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open archive: " + archName);
        }

        // Read and verify archive header
        FileHeader header;
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(header)) ||
            header.signature != SIGNATURE) {
            throw std::runtime_error("Invalid archive format");
        }

        // Read file entries
        while (file) {
            if (!file.read(reinterpret_cast<char*>(&header), sizeof(header)))
                break;

            if (header.signature != SIGNATURE)
                break;

            std::string fileName;
            fileName.resize(header.nameLength);
            if (!file.read(&fileName[0], header.nameLength))
                break;

            // Store entry information
            entries.push_back(ArchiveEntry{
                fileName,
                header.compressedSize,
                header.originalSize,
                header.timestamp
            });

            // Skip compressed data
            file.seekg(header.compressedSize, std::ios::cur);
        }
    }
}
void Archive::createSelfExtracting(const std::vector<std::filesystem::path>& files,
                                  const std::string& outputPath,
                                  CompressionType compression,
                                  const AutoExecConfig& autoExec,
                                  const std::string& stubPath) {
    if (files.empty()) {
        throw std::runtime_error("No files specified for self-extracting archive");
    }

    // Step 1: Create archive data in memory
    std::ostringstream archiveStream;
    entries.clear();

    // Find common base path for all files
    std::filesystem::path basePath;
    if (files.size() > 1) {
        basePath = files[0].parent_path();
        for (const auto& file : files) {
            auto parent = file.parent_path();
            while (!basePath.empty() && !std::filesystem::equivalent(basePath, parent) && 
                   !std::filesystem::equivalent(basePath, parent.parent_path())) {
                basePath = basePath.parent_path();
            }
        }
    }

    // Write archive header
    FileHeader header{};
    header.signature = SIGNATURE;
    header.version = CURRENT_VERSION;
    archiveStream.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Add files to archive stream
    for (const auto& file : files) {
        if (!std::filesystem::exists(file)) {
            throw std::runtime_error("File not found: " + file.string());
        }

        if (!std::filesystem::is_regular_file(file)) {
            std::cerr << "Skipping non-regular file: " << file << std::endl;
            continue;
        }

        std::string relativePath = makeArchivePath(file, basePath);
        addFileToArchiveStream(file, relativePath, archiveStream, compression);
    }

    // Convert stream to vector
    std::string archiveString = archiveStream.str();
    std::vector<char> archiveData(archiveString.begin(), archiveString.end());

    // Step 2: Get or build extractor stub
    std::string actualStubPath = stubPath;
    if (actualStubPath.empty()) {
        actualStubPath = "extractor_stub.exe";
        if (!buildExtractorStub(actualStubPath)) {
            throw std::runtime_error("Failed to build extractor stub");
        }
    }

    // Step 3: Combine stub with archive data and command config
    if (!combineStubWithArchive(actualStubPath, archiveData, outputPath, autoExec)) {
        throw std::runtime_error("Failed to create self-extracting executable");
    }

    std::cout << "Self-extracting executable '" << outputPath 
              << "' created successfully with " << entries.size() << " files." << std::endl;
    
    if (!autoExec.command.empty()) {
        std::cout << "Auto-execution configured: " << autoExec.command;
        if (!autoExec.arguments.empty()) {
            std::cout << " " << autoExec.arguments;
        }
        std::cout << std::endl;
    }
}

bool Archive::buildExtractorStub(const std::string& outputPath) {
    std::cout << "Building extractor stub..." << std::endl;
    
    // This assumes you have the extractor_stub.cpp in your project
    // and a C++ compiler available
    std::string compileCommand;
    
#ifdef _WIN32
    // Try to use cl.exe (Visual Studio) or g++ (MinGW)
    compileCommand = "cl.exe /Fe:" + outputPath + " extractor_stub.cpp /EHsc";
    if (std::system("cl.exe >nul 2>&1") != 0) {
        // Fall back to g++
        compileCommand = "g++ -o " + outputPath + " extractor_stub.cpp -static";
    }
#else
    compileCommand = "g++ -o " + outputPath + " extractor_stub.cpp -static";
#endif

    int result = std::system(compileCommand.c_str());
    if (result != 0) {
        std::cerr << "Failed to compile extractor stub. Make sure you have:" << std::endl;
        std::cerr << "1. extractor_stub.cpp in the current directory" << std::endl;
        std::cerr << "2. A C++ compiler (cl.exe or g++) in your PATH" << std::endl;
        return false;
    }

    return std::filesystem::exists(outputPath);
}

bool Archive::combineStubWithArchive(const std::string& stubPath,
                                     const std::vector<char>& archiveData,
                                     const std::string& outputPath,
                                     const AutoExecConfig& autoExec) {
    if (!std::filesystem::exists(stubPath)) {
        std::cerr << "Extractor stub not found: " << stubPath << std::endl;
        return false;
    }

    // Read the stub executable
    std::ifstream stubFile(stubPath, std::ios::binary);
    if (!stubFile) {
        std::cerr << "Failed to open stub file: " << stubPath << std::endl;
        return false;
    }

    std::vector<char> stubData(std::istreambuf_iterator<char>(stubFile), {});
    stubFile.close();

    // Create the output file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to create output file: " << outputPath << std::endl;
        return false;
    }

    // Write stub executable
    outFile.write(stubData.data(), stubData.size());

    // Write marker
    const char marker[] = "ARCHIVE_DATA_START_MARKER_12345";
    outFile.write(marker, sizeof(marker) - 1);
    
    // Prepare command configuration structure (matching the stub's CommandConfig)
    struct CommandConfig {
        char command[512];
        char arguments[512];
        bool silent;
        bool waitForCompletion;
        char workingDir[256];
    };
    
    CommandConfig cmdConfig = {};
    
    // Copy strings safely
    if (!autoExec.command.empty()) {
        strncpy(cmdConfig.command, autoExec.command.c_str(), sizeof(cmdConfig.command) - 1);
    }
    if (!autoExec.arguments.empty()) {
        strncpy(cmdConfig.arguments, autoExec.arguments.c_str(), sizeof(cmdConfig.arguments) - 1);
    }
    if (!autoExec.workingDir.empty()) {
        strncpy(cmdConfig.workingDir, autoExec.workingDir.c_str(), sizeof(cmdConfig.workingDir) - 1);
    }
    
    cmdConfig.silent = autoExec.silent;
    cmdConfig.waitForCompletion = autoExec.waitForCompletion;
    
    // Write command configuration
    outFile.write(reinterpret_cast<const char*>(&cmdConfig), sizeof(cmdConfig));
    
    // Write archive size
    size_t archiveSize = archiveData.size();
    outFile.write(reinterpret_cast<const char*>(&archiveSize), sizeof(archiveSize));

    // Write archive data
    outFile.write(archiveData.data(), archiveData.size());

    outFile.close();

    // Make executable on Unix systems
#ifndef _WIN32
    std::filesystem::permissions(outputPath, 
        std::filesystem::perms::owner_exec | 
        std::filesystem::perms::group_exec | 
        std::filesystem::perms::others_exec,
        std::filesystem::perm_options::add);
#endif

    return true;
}

// Helper method to add file to a stream instead of file
void Archive::addFileToArchiveStream(const std::filesystem::path& file, 
                                    const std::string& archivePath,
                                    std::ostringstream& archive, 
                                    CompressionType compression) {
    // Read the input file
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open input file: " + file.string());
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
    input.close();

    // Compress the data
    auto compressed = compressData(buffer, compression);

    // Write file header
    FileHeader header{};
    header.signature = SIGNATURE;
    header.version = CURRENT_VERSION;
    header.nameLength = static_cast<uint32_t>(archivePath.length());
    header.compressedSize = compressed.size();
    header.originalSize = buffer.size();
    header.timestamp = std::filesystem::last_write_time(file).time_since_epoch().count();

    archive.write(reinterpret_cast<const char*>(&header), sizeof(header));
    archive.write(archivePath.c_str(), header.nameLength);
    archive.write(compressed.data(), header.compressedSize);

    // Store entry information
    entries.push_back(ArchiveEntry{
        archivePath,
        header.compressedSize,
        header.originalSize,
        header.timestamp
    });
}

void Archive::create(const std::vector<fs::path>& files, CompressionType compression) {
    std::ofstream archive(archiveName, std::ios::binary);
    if (!archive) {
        throw std::runtime_error("Failed to create archive: " + archiveName);
    }

    entries.clear();

    // Write dummy header - will be updated with file count later
    FileHeader header{};
    header.signature = SIGNATURE;
    header.version = CURRENT_VERSION;
    archive.write(reinterpret_cast<const char*>(&header), sizeof(header));

    if (files.empty()) {
        archive.close();
        std::cout << "Archive '" << archiveName << "' created successfully (empty)." << std::endl;
        return;
    }

    // Find common base path for all files
    fs::path basePath;
    if (files.size() > 1) {
        basePath = files[0].parent_path();
        for (const auto& file : files) {
            auto parent = file.parent_path();
            while (!basePath.empty() && !fs::equivalent(basePath, parent) && 
                   !fs::equivalent(basePath, parent.parent_path())) {
                basePath = basePath.parent_path();
            }
        }
    }
    for (const auto& file : files) {
        if (!fs::exists(file)) {
            throw std::runtime_error("File not found: " + file.string());
        }

        if (!fs::is_regular_file(file)) {
            std::cerr << "Skipping non-regular file: " << file << std::endl;
            continue;
        }

        std::string relativePath = makeArchivePath(file, basePath);
        addFileToArchive(file, relativePath, archive, compression);
    }

    archive.close();
    std::cout << "Archive '" << archiveName << "' created successfully with " 
              << entries.size() << " files." << std::endl;
}

void Archive::add(const std::vector<fs::path>& files, CompressionType compression) {
    if (files.empty()) {
        throw std::runtime_error("No files specified for adding to archive");
    }

    // Open existing archive in append mode
    std::ofstream archive(archiveName, std::ios::binary | std::ios::app);
    if (!archive) {
        throw std::runtime_error("Failed to open archive for appending: " + archiveName);
    }

    // Find common base path for new files
    fs::path basePath;
    if (files.size() > 1) {
        basePath = files[0].parent_path();
        for (const auto& file : files) {
            auto parent = file.parent_path();
            while (!basePath.empty() && !fs::equivalent(basePath, parent) && 
                   !fs::equivalent(basePath, parent.parent_path())) {
                basePath = basePath.parent_path();
            }
        }
    }

    size_t addedCount = 0;
    for (const auto& file : files) {
        if (!fs::exists(file)) {
            throw std::runtime_error("File not found: " + file.string());
        }

        if (!fs::is_regular_file(file)) {
            std::cerr << "Skipping non-regular file: " << file << std::endl;
            continue;
        }

        std::string relativePath = makeArchivePath(file, basePath);
        addFileToArchive(file, relativePath, archive, compression);
        addedCount++;
    }

    archive.close();
    std::cout << "Added " << addedCount << " files to archive '" << archiveName << "'." << std::endl;
}

std::vector<char> Archive::compressData(const std::vector<char>& input, CompressionType compression) {
    if (input.empty()) return {};

    // Initialize zlib stream
    z_stream strm = {};
    if (deflateInit(&strm, static_cast<int>(compression)) != Z_OK) {
        throw std::runtime_error("Failed to initialize compression");
    }

    // Set up input
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    strm.avail_in = static_cast<uInt>(input.size());

    // Prepare output buffer (slightly larger than input for safety)
    std::vector<char> output(static_cast<size_t>(input.size() * 1.1) + 12);
    strm.next_out = reinterpret_cast<Bytef*>(output.data());
    strm.avail_out = static_cast<uInt>(output.size());

    // Compress
    if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&strm);
        throw std::runtime_error("Compression failed");
    }

    // Clean up and resize output
    deflateEnd(&strm);
    output.resize(strm.total_out);
    return output;
}

void Archive::addFileToArchive(const fs::path& file, const std::string& archivePath,
                             std::ofstream& archive, CompressionType compression) {
    // Read the input file
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open input file: " + file.string());
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
    input.close();

    // Compress the data
    auto compressed = compressData(buffer, compression);

    // Write file header
    FileHeader header{};
    header.signature = SIGNATURE;
    header.version = CURRENT_VERSION;
    header.nameLength = static_cast<uint32_t>(archivePath.length());
    header.compressedSize = compressed.size();
    header.originalSize = buffer.size();
    header.timestamp = fs::last_write_time(file).time_since_epoch().count();

    archive.write(reinterpret_cast<const char*>(&header), sizeof(header));
    archive.write(archivePath.c_str(), header.nameLength);
    archive.write(compressed.data(), header.compressedSize);

    if (!archive) {
        throw std::runtime_error("Failed to write to archive");
    }

    // Store entry information
    entries.push_back(ArchiveEntry{
        archivePath,
        header.compressedSize,
        header.originalSize,
        header.timestamp
    });
}

void Archive::extract(const std::string& outputDir) {
    std::ifstream archive(archiveName, std::ios::binary);
    if (!archive) {
        throw std::runtime_error("Failed to open archive: " + archiveName);
    }

    // Create output directory if it doesn't exist
    fs::path outPath(outputDir);
    if (!fs::exists(outPath)) {
        fs::create_directories(outPath);
    }

    // Read and verify archive header
    FileHeader header;
    if (!archive.read(reinterpret_cast<char*>(&header), sizeof(header)) ||
        header.signature != SIGNATURE) {
        throw std::runtime_error("Invalid archive format");
    }

    while (archive) {
        if (!archive.read(reinterpret_cast<char*>(&header), sizeof(header)))
            break;

        if (header.signature != SIGNATURE)
            break;

        std::string fileName;
        fileName.resize(header.nameLength);
        if (!archive.read(&fileName[0], header.nameLength))
            break;

        // Create full output path, including any subdirectories
        fs::path fullPath = outPath / fileName;
        fs::create_directories(fullPath.parent_path());

        // Read compressed data
        std::vector<char> compressedData(header.compressedSize);
        if (!archive.read(compressedData.data(), header.compressedSize))
            break;

        // Decompress
        z_stream strm = {};
        if (inflateInit(&strm) != Z_OK) {
            throw std::runtime_error("Failed to initialize decompression");
        }

        std::vector<char> decompressedData(header.originalSize);
        strm.next_in = reinterpret_cast<Bytef*>(compressedData.data());
        strm.avail_in = static_cast<uInt>(header.compressedSize);
        strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());
        strm.avail_out = static_cast<uInt>(header.originalSize);

        if (inflate(&strm, Z_FINISH) != Z_STREAM_END) {
            inflateEnd(&strm);
            throw std::runtime_error("Decompression failed for: " + fileName);
        }
        inflateEnd(&strm);

        // Write decompressed data
        std::ofstream outFile(fullPath, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to create output file: " + fullPath.string());
        }
        outFile.write(decompressedData.data(), header.originalSize);
        outFile.close();

        // Set file timestamp using filesystem operations
        auto ft = fs::file_time_type(fs::file_time_type::duration(header.timestamp));
        fs::last_write_time(fullPath, ft);
    }

    archive.close();
    std::cout << "Archive '" << archiveName << "' extracted successfully to '" 
              << outputDir << "'." << std::endl;
}
