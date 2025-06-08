/**
 * @file Version.h
 * @brief Version information for ModernArchive
 * 
 * This file maintains version information and tracks the evolution from
 * the original Sonic Foundry archiver to this modern reimplementation.
 */

#pragma once

namespace ModernArchive {

constexpr int MAJOR_VERSION = 2;     ///< Major version number
constexpr int MINOR_VERSION = 0;     ///< Minor version number
constexpr int PATCH_VERSION = 0;     ///< Patch level

constexpr const char* VERSION_STRING = "2.0.0";
constexpr const char* PROJECT_NAME = "ModernArchive";
constexpr const char* PROJECT_DESCRIPTION = "Modern Cross-Platform Archive Utility";

/**
 * Version history:
 * 
 * 1.0 (Original Sonic Foundry Version, late 1990s)
 * - Windows-only implementation
 * - Custom compression algorithms
 * - Self-extracting executable format
 * - GUI-based extraction
 * 
 * 2.0 (Modern Reimplementation, 2025)
 * - Cross-platform support
 * - ZLIB compression
 * - Modern C++17 implementation
 * - CMake build system
 * - Comprehensive testing
 * - Thread-safe operations
 */

} // namespace ModernArchive
