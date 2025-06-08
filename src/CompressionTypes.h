#ifndef COMPRESSION_TYPES_H
#define COMPRESSION_TYPES_H

#include <zlib.h>

/**
 * @brief Available compression algorithms
 */
enum class CompressionType {
    None,       ///< No compression
    Deflate,    ///< ZLIB deflate algorithm
    LZMA,       ///< LZMA compression (future)
    BZIP2       ///< BZIP2 compression (future)
};

/**
 * @brief Compression level options
 */
enum class CompressionLevel {
    None,       ///< No compression
    Fastest = Z_BEST_SPEED,    ///< Fastest compression (less compression ratio)
    Fast,       ///< Fast compression
    Normal = Z_DEFAULT_COMPRESSION,     ///< Normal compression (default)
    Maximum = Z_BEST_COMPRESSION     ///< Maximum compression (slower)
};

/**
 * @brief Flags for file addition operations
 */
enum class AddFlags {
    None = 0,       ///< No special behavior
    Update = 1,     ///< Update existing files
    Force = 2       ///< Force operation
};

#endif // COMPRESSION_TYPES_H
