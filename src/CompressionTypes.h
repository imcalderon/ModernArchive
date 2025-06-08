#ifndef COMPRESSION_TYPES_H
#define COMPRESSION_TYPES_H

#include <zlib.h>

/**
 * @brief Compression level for ZLIB
 * Using ZLIB constants directly for compatibility
 */
enum class CompressionType : int {
    Fastest = Z_BEST_SPEED,         ///< Fastest compression (1)
    Fast = 3,                       ///< Fast compression
    Normal = Z_DEFAULT_COMPRESSION, ///< Normal compression (-1/6)
    Best = Z_BEST_COMPRESSION       ///< Maximum compression (9)
};

#endif // COMPRESSION_TYPES_H