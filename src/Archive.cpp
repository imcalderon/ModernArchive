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

namespace fs = std::filesystem;

// Helper function to make paths relative and convert to archive format
std::string makeArchivePath(const fs::path& file, const fs::path& basePath = {}) {
    std::string result;
    if (basePath.empty()) {
        result = file.filename().string();
    } else {
        result = fs::relative(file, basePath).string();
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
