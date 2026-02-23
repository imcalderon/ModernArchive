# ModernArchive

A modern, cross-platform archive utility with self-extracting executable support.

## ✨ Key Features

- **📦 Archive Management**: Create, extract, and manage compressed archives.
- **🚀 Self-Extracting Executables**: Generate standalone `.exe` files for software distribution.
- **⚡ Smart Compression**: Multiple compression levels using industry-standard ZLIB.
- **🔄 Auto-Execution**: Automatically run installers (MSI, setup.exe) after extraction.
- **🌐 Cross-Platform**: Native support for Windows (MSVC), Linux, and macOS.
- **⚙️ Modern Implementation**: C++17, CMake build system.

## 🚀 Build Instructions (Windows / MSVC)

ModernArchive requires a C++17 compiler and ZLIB.

### Prerequisites
- **Visual Studio 2022** (with C++ Build Tools)
- **CMake** and **Ninja**
- **ZLIB** (available via Conda or vcpkg)

### Build Steps

```powershell
# 1. Activate your Visual Studio environment (vcvars64.bat)
# 2. Configure and build
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/path/to/zlib"
cmake --build .
```

If using **DevEnv/Conda**, the prefix path is usually `~/miniconda3/Library`.

## 🎪 Self-Extracting Archive Example

```bash
# Create installer that automatically runs an MSI package
archive selfext MyApp-Setup.exe \
    --exec msiexec \
    --args "/i MyApp.msi /quiet" \
    MyApp.msi dependency.dll README.txt
```

## 🛠️ Tech Stack

- **Standard**: C++17
- **Compression**: ZLIB
- **Build System**: CMake 3.12+
- **Platform Support**: Windows (cl.exe), Linux/macOS (g++, clang)

## 📄 License

This project is licensed under the MIT License.
