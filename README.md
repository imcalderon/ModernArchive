# ModernArchive

A modern, cross-platform reimplementation of and old self-extracting archive tool, rebuilt for today's development landscape with the assistance of AI.

## Historical Context

This project is a modern reimagining of a self-extracting archive tool originally developed at Sonic Foundry in the late 1990s. The original tool was used to package and distribute software products, including the popular Sound Forge audio editing software.

### The Original Implementation

The original Sonic Foundry archiver was a Windows-based tool that:
- Created self-extracting executables for software distribution
- Used custom compression algorithms optimized for audio software packages
- Included a GUI-based extraction interface with progress monitoring
- Supported selective file extraction and verification

## Modern Reimplementation

This new implementation maintains the spirit of the original while embracing modern development practices and cross-platform compatibility. Key improvements include:

- Cross-platform support (Windows, Linux, macOS)
- Industry-standard ZLIB compression
- Modern C++17 features
- Comprehensive test coverage
- CMake-based build system
- Thread-safe operations

## Features

- Create and extract archive files
- ZLIB-based compression with multiple compression levels
- Progress tracking during operations
- Robust error handling
- Header-based archive format
- File integrity verification
- Cross-platform compatibility

## Building

### Prerequisites

- CMake 3.12 or higher
- C++17 compliant compiler
- ZLIB development libraries
- Google Test (for building tests)
- vcpkg (recommended for dependency management)

### Build Instructions

```powershell
# Clone the repository
git clone https://github.com/yourusername/ModernArchive.git
cd ModernArchive

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg]/scripts/buildsystems/vcpkg.cmake `
      -DBUILD_TESTS=ON ..

# Build
cmake --build . --config Release
```

### Running Tests

```powershell
cd build
ctest -C Release --output-on-failure
```

## Usage

### Command Line Interface

```bash
# Create an archive
archive create output.arc file1.txt file2.txt

# Extract an archive
archive extract archive.arc [destination_directory]

# List contents
archive list archive.arc

# Add files to existing archive
archive add archive.arc newfile.txt
```

### Library Usage

```cpp
#include <archive/Archive.h>

// Create an archive
Archive archive("output.arc");
std::vector<std::string> files = {"file1.txt", "file2.txt"};
archive.createArchive(files);

// Extract an archive
archive.extractArchive("destination_directory");
```

## Archive Format

The archive format uses a simple but robust structure:

```
Archive File Structure:
+----------------+
| File Header    |
|  - Signature   |
|  - Version     |
|  - Name Length |
|  - Sizes      |
|  - Timestamp   |
+----------------+
| File Name      |
+----------------+
| Compressed     |
| File Data      |
+----------------+
| Next File...   |
+----------------+
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Original Sonic Foundry development team
- Modern reimplementation assisted by GitHub Copilot
- ZLIB compression library
- Google Test framework

## About this Reimplementation

This modern version was created with the assistance of GitHub Copilot, demonstrating how AI can help preserve and modernize historical software engineering knowledge. The project maintains the core functionality of the original Sonic Foundry tool while adding modern features and cross-platform support.

The reimplementation process focused on:
1. Preserving the original tool's reliability and performance
2. Adding modern programming practices and error handling
3. Ensuring cross-platform compatibility
4. Implementing comprehensive testing
5. Providing clear documentation and examples

This project serves as both a functional tool and a historical bridge between classic Windows software development and modern cross-platform practices.