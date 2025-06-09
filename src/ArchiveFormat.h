#pragma once

#include <cstdint>
#include <ctime>

// Archive format constants
constexpr uint32_t SIGNATURE = 0x4E415649;  // "IVAN"
constexpr uint16_t CURRENT_VERSION = 0x0200; // Version 2.0

// Header structure for each file in the archive
struct FileHeader {
    uint32_t signature;      // File signature "IVAN"
    uint16_t version;        // Archive version
    uint32_t nameLength;     // Length of the file name
    uint64_t compressedSize; // Size after compression
    uint64_t originalSize;   // Original file size
    int64_t timestamp;       // File timestamp
};