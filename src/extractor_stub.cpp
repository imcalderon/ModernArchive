// extractor_stub.cpp - Self-extracting archive stub with auto-execution support
// This gets compiled into a small executable that can extract embedded archive data
// and optionally execute a command on the extracted files

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <zlib.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

// Magic marker to find where archive data starts
const char ARCHIVE_MARKER[] = "ARCHIVE_DATA_START_MARKER_12345";
const size_t MARKER_SIZE = sizeof(ARCHIVE_MARKER) - 1;

// Command configuration structure
struct CommandConfig {
    char command[512];      // Command to execute
    char arguments[512];    // Command arguments
    bool silent;            // Run silently
    bool waitForCompletion; // Wait for command to complete
    char workingDir[256];   // Working directory (empty = extraction dir)
};

bool findArchiveData(std::ifstream& file, size_t& archiveOffset, size_t& archiveSize, CommandConfig& cmdConfig) {
    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read the entire file into memory (for small executables this is fine)
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    
    // Search for the marker from the end of the file
    for (size_t i = fileSize - MARKER_SIZE; i > 0; --i) {
        if (std::memcmp(buffer.data() + i, ARCHIVE_MARKER, MARKER_SIZE) == 0) {
            // Read command config (stored right after marker)
            std::memcpy(&cmdConfig, buffer.data() + i + MARKER_SIZE, sizeof(CommandConfig));
            
            // Read archive size (stored after command config)
            std::memcpy(&archiveSize, buffer.data() + i + MARKER_SIZE + sizeof(CommandConfig), sizeof(size_t));
            
            // Archive data starts after size field
            archiveOffset = i + MARKER_SIZE + sizeof(CommandConfig) + sizeof(size_t);
            return true;
        }
    }
    return false;
}

bool executeCommand(const CommandConfig& cmdConfig, const std::string& extractDir) {
    if (strlen(cmdConfig.command) == 0) {
        return true; // No command to execute
    }
    
    std::string workDir = strlen(cmdConfig.workingDir) > 0 ? cmdConfig.workingDir : extractDir;
    std::string fullCommand = cmdConfig.command;
    
    if (strlen(cmdConfig.arguments) > 0) {
        fullCommand += " ";
        fullCommand += cmdConfig.arguments;
    }
    
    std::cout << "Executing: " << fullCommand << std::endl;
    std::cout << "Working directory: " << workDir << std::endl;
    
#ifdef _WIN32
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    
    // Set working directory
    const char* workingDir = workDir.c_str();
    
    // Create process
    BOOL success = CreateProcessA(
        NULL,                           // No module name (use command line)
        const_cast<char*>(fullCommand.c_str()), // Command line
        NULL,                           // Process handle not inheritable
        NULL,                           // Thread handle not inheritable
        FALSE,                          // Set handle inheritance to FALSE
        cmdConfig.silent ? CREATE_NO_WINDOW : 0, // Creation flags
        NULL,                           // Use parent's environment block
        workingDir,                     // Working directory
        &si,                            // Pointer to STARTUPINFO structure
        &pi                             // Pointer to PROCESS_INFORMATION structure
    );
    
    if (!success) {
        std::cerr << "Error: Failed to execute command. Error code: " << GetLastError() << std::endl;
        return false;
    }
    
    if (cmdConfig.waitForCompletion) {
        std::cout << "Waiting for command to complete..." << std::endl;
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        std::cout << "Command completed with exit code: " << exitCode << std::endl;
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
#else
    // Unix/Linux implementation
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (chdir(workDir.c_str()) != 0) {
            std::cerr << "Error: Failed to change directory to " << workDir << std::endl;
            exit(1);
        }
        
        // Execute command
        execl("/bin/sh", "sh", "-c", fullCommand.c_str(), NULL);
        std::cerr << "Error: Failed to execute command" << std::endl;
        exit(1);
    } else if (pid > 0) {
        // Parent process
        if (cmdConfig.waitForCompletion) {
            int status;
            waitpid(pid, &status, 0);
            std::cout << "Command completed with status: " << WEXITSTATUS(status) << std::endl;
        }
    } else {
        std::cerr << "Error: Failed to fork process" << std::endl;
        return false;
    }
#endif
    
    return true;
}

bool extractArchive(const std::string& executablePath, const std::string& outputDir) {
    std::ifstream file(executablePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open executable file" << std::endl;
        return false;
    }
    
    size_t archiveOffset, archiveSize;
    CommandConfig cmdConfig = {};
    if (!findArchiveData(file, archiveOffset, archiveSize, cmdConfig)) {
        std::cerr << "Error: No archive data found in executable" << std::endl;
        return false;
    }
    
    // Create output directory
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }
    
    // Extract archive data to temporary file
    fs::path tempArchive = fs::temp_directory_path() / "temp_extract.arc";
    {
        std::ofstream tempFile(tempArchive, std::ios::binary);
        if (!tempFile) {
            std::cerr << "Error: Cannot create temporary file" << std::endl;
            return false;
        }
        
        file.seekg(archiveOffset);
        std::vector<char> archiveData(archiveSize);
        file.read(archiveData.data(), archiveSize);
        tempFile.write(archiveData.data(), archiveSize);
    }
    
    // Now extract using the Archive class logic (simplified version)
    // This is a minimal implementation - you'd want to include the full Archive extraction logic
    std::ifstream archive(tempArchive, std::ios::binary);
    if (!archive) {
        std::cerr << "Error: Cannot read extracted archive data" << std::endl;
        return false;
    }
    
    // Read archive format (simplified - this should match your Archive.cpp format)
    struct FileHeader {
        uint32_t signature;
        uint16_t version; 
        uint32_t nameLength;
        uint64_t compressedSize;
        uint64_t originalSize;
        int64_t timestamp;
    };
    
    const uint32_t SIGNATURE = 0x4E415649; // "IVAN"
    
    // Skip the first header (archive header)
    FileHeader header;
    archive.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    int filesExtracted = 0;
    while (archive) {
        if (!archive.read(reinterpret_cast<char*>(&header), sizeof(header)))
            break;
            
        if (header.signature != SIGNATURE)
            break;
            
        // Read filename
        std::string fileName(header.nameLength, '\0');
        if (!archive.read(&fileName[0], header.nameLength))
            break;
            
        // Create output file path
        fs::path outputPath = fs::path(outputDir) / fileName;
        fs::create_directories(outputPath.parent_path());
        
        // Read compressed data
        std::vector<char> compressedData(header.compressedSize);
        if (!archive.read(compressedData.data(), header.compressedSize))
            break;
            
        // Decompress data using zlib
        std::vector<char> decompressedData;
        
        if (header.originalSize == header.compressedSize) {
            // Data is not compressed
            decompressedData = compressedData;
        } else {
            // Decompress using zlib
            decompressedData.resize(header.originalSize);
            z_stream strm = {};
            
            if (inflateInit(&strm) != Z_OK) {
                std::cerr << "Error: Failed to initialize decompression for " << fileName << std::endl;
                continue;
            }
            
            strm.next_in = reinterpret_cast<Bytef*>(compressedData.data());
            strm.avail_in = static_cast<uInt>(header.compressedSize);
            strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());
            strm.avail_out = static_cast<uInt>(header.originalSize);
            
            if (inflate(&strm, Z_FINISH) != Z_STREAM_END) {
                inflateEnd(&strm);
                std::cerr << "Error: Decompression failed for " << fileName << std::endl;
                continue;
            }
            inflateEnd(&strm);
        }
        
        // Write decompressed data
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
            continue;
        }
        
        outFile.write(decompressedData.data(), header.originalSize);
        
        filesExtracted++;
    }
    
    // Clean up temporary file
    fs::remove(tempArchive);
    
    std::cout << "Successfully extracted " << filesExtracted << " files to " << outputDir << std::endl;
    
    // Execute command if specified
    if (strlen(cmdConfig.command) > 0) {
        std::cout << std::endl;
        if (!executeCommand(cmdConfig, outputDir)) {
            std::cerr << "Warning: Command execution failed" << std::endl;
            return false;
        }
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "ModernArchive Self-Extractor" << std::endl;
    
    std::string outputDir = ".";
    bool silentMode = false;
    bool skipExecution = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--silent" || arg == "-s") {
            silentMode = true;
        } else if (arg == "--skip-exec" || arg == "-n") {
            skipExecution = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options] [output_directory]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --silent, -s     Run in silent mode (minimal output)" << std::endl;
            std::cout << "  --skip-exec, -n  Skip automatic command execution" << std::endl;
            std::cout << "  --help, -h       Show this help message" << std::endl;
            return 0;
        } else if (!arg.empty() && arg[0] != '-') {
            outputDir = arg;
        }
    }
    
    if (!silentMode) {
        std::cout << "Extracting to: " << outputDir << std::endl;
    }
    // Get path to current executable
    std::string executablePath;
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    executablePath = buffer;
#else
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        executablePath = buffer;
    }
#endif
    
    if (executablePath.empty()) {
        std::cerr << "Error: Cannot determine executable path" << std::endl;
        return 1;
    }
    
    if (!extractArchive(executablePath, outputDir)) {
        std::cerr << "Extraction failed" << std::endl;
        return 1;
    }
    
    if (!silentMode) {
        std::cout << "Extraction completed successfully!" << std::endl;
    }
    
    return 0;
}