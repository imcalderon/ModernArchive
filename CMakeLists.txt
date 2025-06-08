cmake_minimum_required(VERSION 3.12)

project(ModernArchive 
    VERSION 2.0.0
    DESCRIPTION "Modern Cross-Platform Archive Utility"
    LANGUAGES CXX)

# Configure version information
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h"
    @ONLY
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Build type specific options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
    if(NOT MSVC)
        add_compile_options(-g -O0)
    endif()
else()
    add_compile_definitions(NDEBUG=1)
    if(NOT MSVC)
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Find required packages
find_package(ZLIB REQUIRED)

# Optional: Find threads for future parallel processing
find_package(Threads)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(ARCHIVE_SOURCES
    src/Archive.cpp
    src/ArchiveConsole.cpp
    src/ArchiveProgress.cpp
    src/CrossPlatform.cpp
)

set(ARCHIVE_HEADERS
    src/ArchiveConsole.h
    src/ArchiveProgress.h
    src/CrossPlatform.h
    src/Archive.h
    src/CompressionTypes.h
    src/ArchiveFormat.h
    src/Version.h
)

# Create the library first (used by both main executable and tests)
add_library(libarchive STATIC
    ${ARCHIVE_SOURCES}
    ${ARCHIVE_HEADERS}
)

# Set output name without 'lib' prefix on Windows
set_target_properties(libarchive PROPERTIES
    OUTPUT_NAME "archive"
    PREFIX ""
)

# Link ZLIB to the library
target_link_libraries(libarchive PUBLIC ZLIB::ZLIB)

if(Threads_FOUND)
    target_link_libraries(libarchive PUBLIC Threads::Threads)
endif()

# Platform-specific linking for library
if(WIN32)
    target_link_libraries(libarchive PUBLIC ws2_32 shlwapi)
elseif(UNIX)
    target_link_libraries(libarchive PUBLIC dl)
    if(NOT APPLE)
        target_link_libraries(libarchive PUBLIC rt)
    endif()
endif()

# Create the executable
add_executable(archive src/main.cpp)
target_link_libraries(archive PRIVATE libarchive)

# Set version information for Windows executables
if(WIN32)
    set_target_properties(archive PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        RUNTIME_OUTPUT_NAME "archive-${PROJECT_VERSION}"
    )
endif()

# Build examples
add_executable(example_usage examples/example_usage.cpp)
target_link_libraries(example_usage PRIVATE libarchive)
target_include_directories(example_usage PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(WIN32)
    set_target_properties(example_usage PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

# Tests configuration
option(BUILD_TESTS "Build the test suite" ON)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    
    # Add test executable
    add_executable(archive_tests
        tests/test_archive.cpp
        tests/test_compression.cpp
    )
    
    target_include_directories(archive_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    
    target_link_libraries(archive_tests
        PRIVATE
        libarchive
        GTest::GTest
        GTest::Main
    )
    
    # Register tests
    include(GoogleTest)
    gtest_discover_tests(archive_tests)
endif()

# Installation
install(TARGETS archive libarchive example_usage
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES README.md LICENSE
    DESTINATION share/doc/archive
    COMPONENT documentation
)

install(FILES ${ARCHIVE_HEADERS}
    DESTINATION include/archive
    COMPONENT development
)