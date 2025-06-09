# ModernArchive

A modern, cross-platform archive utility with self-extracting executable support. This is a complete reimplementation an old Sonic Foundry archive tool from the late 1990s, rebuilt for today's development landscape with modern C++17 and cross-platform compatibility.

## ✨ Key Features

- **📦 Archive Management**: Create, extract, and manage compressed archives
- **🚀 Self-Extracting Executables**: Generate standalone .exe files for software distribution
- **⚡ Smart Compression**: Multiple compression levels using industry-standard ZLIB
- **🔄 Auto-Execution**: Automatically run installers (MSI, setup.exe) after extraction
- **🌐 Cross-Platform**: Native support for Windows, Linux, and macOS
- **📁 Directory Support**: Preserves complete directory structures
- **🛡️ Data Integrity**: Built-in file verification and robust error handling
- **⚙️ Modern Implementation**: C++17, CMake build system, comprehensive testing

## 🎯 Use Cases

### Software Distribution
Create self-extracting installers that automatically launch MSI packages, setup executables, or custom installation scripts.

### File Packaging
Bundle multiple files and directories into compressed archives for easy distribution and storage.

### Automated Deployment
Generate executables that extract and automatically configure software in deployment scenarios.

## 🚀 Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/ModernArchive.git
cd ModernArchive

# Create build directory
mkdir build && cd build

# Configure with CMake (Windows with vcpkg)
cmake -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg]/scripts/buildsystems/vcpkg.cmake \
      -DBUILD_TESTS=ON ..

# Build
cmake --build . --config Release

# Run tests
ctest -C Release --output-on-failure
```

### Basic Usage Examples

```bash
# Create an archive from files
archive create myarchive.arc file1.txt file2.txt folder/

# Extract an archive
archive extract myarchive.arc output_folder/

# List archive contents
archive list myarchive.arc

# Add files to existing archive
archive add myarchive.arc newfile.txt
```

## 📦 Archive Operations

### Creating Archives

```bash
# Basic archive creation
archive create backup.arc documents/ photos/ important.txt

# With compression options
archive create --best data.arc large_files/     # Maximum compression
archive create --fastest temp.arc logs/         # Speed over size
archive create --normal docs.arc documents/     # Balanced (default)
```

### Extracting Archives

```bash
# Extract to current directory
archive extract backup.arc

# Extract to specific directory
archive extract backup.arc /path/to/destination/

# The tool preserves the original directory structure
```

### Managing Archives

```bash
# View detailed contents
archive list backup.arc
# Output shows:
# Name                                              Size      Compressed
# ------------------------------------------------------------
# documents/report.pdf                             1048576      524288
# photos/vacation.jpg                              2097152     2050000
# important.txt                                       1024         512

# Add more files to existing archive
archive add backup.arc new_document.pdf recent_photos/
```

## 🎪 Self-Extracting Executables

The standout feature of ModernArchive is creating self-extracting executables that can automatically run installers or commands after extraction.

### Basic Self-Extracting Archive

```bash
# Create a simple self-extracting executable
archive selfext installer.exe setup_files/ documentation/
```

### MSI Installer Distribution

Perfect for distributing Windows software packages:

```bash
# Create installer that automatically runs MSI package
archive selfext MyApp-Setup.exe \
    --exec msiexec \
    --args "/i MyApp.msi /quiet" \
    --silent \
    MyApp.msi documentation/ resources/

# When users run MyApp-Setup.exe:
# 1. Files extract to temporary/chosen directory
# 2. msiexec automatically runs with specified arguments
# 3. MSI installs silently in background
```

### Advanced Self-Extracting Options

```bash
# Full-featured installer
archive selfext enterprise-setup.exe \
    --exec setup.exe \
    --args "--install-path 'C:\Program Files\MyApp' --silent" \
    --workdir "C:\temp\install" \
    --no-wait \
    setup.exe config/ drivers/ documentation/

# Custom extractor stub
archive selfext custom.exe \
    --stub my_custom_extractor.exe \
    --exec post_install.bat \
    files_to_package/
```

### Self-Extractor Options Reference

| Option | Description | Example |
|--------|-------------|---------|
| `--exec <command>` | Command to run after extraction | `--exec msiexec` |
| `--args <arguments>` | Arguments for the command | `--args "/i app.msi /quiet"` |
| `--silent` | Run command without showing window | `--silent` |
| `--no-wait` | Don't wait for command completion | `--no-wait` |
| `--workdir <dir>` | Working directory for command | `--workdir "C:\temp"` |
| `--stub <path>` | Use custom extractor executable | `--stub custom_stub.exe` |

### Running Self-Extracting Files

End users can run the generated executables with options:

```bash
# Extract to current directory and run auto-command
MyApp-Setup.exe

# Extract to specific directory
MyApp-Setup.exe "C:\MyApp"

# Extract without running auto-command
MyApp-Setup.exe --skip-exec "C:\MyApp"

# Silent extraction
MyApp-Setup.exe --silent "C:\MyApp"
```

## 🛠️ Library Usage

Integrate ModernArchive into your C++ applications:

```cpp
#include <archive/Archive.h>

// Create an archive
Archive archive("output.arc");
std::vector<std::filesystem::path> files = {"file1.txt", "folder/"};
archive.create(files, CompressionType::Best);

// Extract an archive
archive.extract("destination_directory");

// Create self-extracting executable with auto-execution
AutoExecConfig autoExec;
autoExec.command = "msiexec";
autoExec.arguments = "/i installer.msi /quiet";
autoExec.silent = true;

archive.createSelfExtracting(files, "installer.exe", 
                           CompressionType::Normal, autoExec);

// List archive contents
auto entries = archive.getFileList();
for (const auto& entry : entries) {
    std::cout << entry.name << " (" << entry.originalSize << " bytes)\n";
}
```

## 📋 Requirements

### Build Requirements
- **CMake**: 3.12 or higher
- **C++ Compiler**: C++17 compliant (Visual Studio 2019+, GCC 7+, Clang 6+)
- **ZLIB**: Development libraries
- **Google Test**: For building tests (optional)

### Runtime Requirements
- **Windows**: Windows 7 or later
- **Linux**: Most modern distributions
- **macOS**: macOS 10.14 or later

### Dependency Management
We recommend using [vcpkg](https://github.com/Microsoft/vcpkg) for dependency management:

```bash
# Install dependencies with vcpkg
vcpkg install zlib gtest
```

## 🏗️ Build Configurations

### Windows (Visual Studio)
```powershell
cmake -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_TOOLCHAIN_FILE=[vcpkg_path]/scripts/buildsystems/vcpkg.cmake \
      -DBUILD_TESTS=ON ..
cmake --build . --config Release
```

### Linux/macOS
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
make -j$(nproc)
```

### Build Options
- `BUILD_TESTS=ON/OFF` - Enable/disable test suite compilation
- `CMAKE_BUILD_TYPE` - Debug, Release, RelWithDebInfo, MinSizeRel

## 🧪 Testing

```bash
# Run all tests
ctest -C Release --output-on-failure

# Run specific test categories
ctest -C Release -R "Compression"
ctest -C Release -R "Archive"

# Test with example data
cd examples
powershell ./test_features.ps1  # Windows
./test_features.sh              # Linux/macOS
```

## 📁 Archive Format

ModernArchive uses a custom but straightforward binary format:

```
Archive Structure:
┌─────────────────┐
│ File Header     │ ← IVAN signature, version
├─────────────────┤
│ File Entry 1    │ ← Name length, sizes, timestamp
│ ├─ File Name    │
│ └─ Compressed   │
│    Data         │
├─────────────────┤
│ File Entry 2    │
│ ...             │
└─────────────────┘
```

- **Signature**: "MARC" (0x4352414D)
- **Version**: 2.0 (0x0200)
- **Compression**: ZLIB deflate
- **Cross-platform**: Forward slash path separators

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow C++17 standards
- Add tests for new features
- Update documentation for API changes
- Ensure cross-platform compatibility

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🏆 Acknowledgments

- **Original Sonic Foundry Team**: For the great tools that inspired this project
- **Modern Development**: Reimplemented with assistance from AI tools
- **Open Source Libraries**: ZLIB for compression, Google Test for testing framework
- **Community**: Contributors and users who help improve the project

## 📚 Additional Resources

- [Examples and Tutorials](examples/)
---

**ModernArchive**: Bridging classic software distribution with modern development practices. 🚀