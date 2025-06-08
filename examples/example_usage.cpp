#include <iostream>
#include <filesystem>
#include "Archive.h"
#include "ArchiveConsole.h"
#include "Version.h"

namespace fs = std::filesystem;

void printVersion() {
    std::cout << ModernArchive::PROJECT_NAME << " version " << ModernArchive::VERSION_STRING << "\n"
              << ModernArchive::PROJECT_DESCRIPTION << "\n"
              << "Build date: " << __DATE__ << " " << __TIME__ << "\n";
}

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <command> <archive_name> [options]\n\n"
              << "Commands:\n"
              << "  create   - Create a new archive: " << programName << " create <archive_name> <file1> [file2 ...]\n"
              << "  extract  - Extract an archive: " << programName << " extract <archive_name> [output_directory]\n"
              << "  list     - List archive contents: " << programName << " list <archive_name>\n"
              << "  add      - Add files to archive: " << programName << " add <archive_name> <file1> [file2 ...]\n"
              << "  version  - Show version information\n\n"
              << "Compression Options:\n"
              << "  --fastest  Use fastest compression\n"
              << "  --best    Use best compression\n"
              << "  --normal  Use normal compression (default)\n";
}

bool ensureDirectoryExists(const fs::path& path) {
    std::error_code ec;
    if (!fs::exists(path)) {
        if (!fs::create_directories(path, ec)) {
            std::cerr << "Error creating directory " << path << ": " << ec.message() << "\n";
            return false;
        }
    }
    return true;
}

fs::path makeAbsolute(const fs::path& path) {
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if (ec) {
        std::cerr << "Error resolving path " << path << ": " << ec.message() << "\n";
        return path;
    }
    return absolute;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    
    if (command == "version") {
        printVersion();
        return 0;
    }

    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    fs::path archivePath = makeAbsolute(argv[2]);
    
    // Ensure the archive's parent directory exists for create/add commands
    if (command == "create" || command == "add") {
        if (!ensureDirectoryExists(archivePath.parent_path())) {
            return 1;
        }
    }

    Archive archive(archivePath.string());

    try {
        if (command == "create" || command == "add") {
            CompressionType compression = CompressionType::Normal;
            std::vector<fs::path> files;
            
            for (int i = 3; i < argc; i++) {
                std::string arg = argv[i];
                if (arg == "--fastest") compression = CompressionType::Fastest;
                else if (arg == "--best") compression = CompressionType::Best;
                else if (arg == "--normal") compression = CompressionType::Normal;
                else {
                    fs::path inputPath = makeAbsolute(arg);
                    if (!fs::exists(inputPath)) {
                        std::cerr << "Error: Path not found: " << inputPath << "\n";
                        return 1;
                    }
                    
                    if (fs::is_directory(inputPath)) {
                        // Add all files from the directory recursively
                        for (const auto& entry : fs::recursive_directory_iterator(inputPath)) {
                            if (fs::is_regular_file(entry)) {
                                files.push_back(entry.path());
                            }
                        }
                    } else {
                        files.push_back(inputPath);
                    }
                }
            }

            if (files.empty()) {
                std::cerr << "Error: No input files found\n";
                return 1;
            }

            if (command == "create") {
                // Sort files to ensure consistent order
                std::sort(files.begin(), files.end());
                archive.create(files, compression);
            } else {
                archive.add(files, compression);
            }
        }
        else if (command == "extract") {
            fs::path outputDir = argc > 3 ? makeAbsolute(argv[3]) : fs::current_path();
            
            if (!ensureDirectoryExists(outputDir)) {
                return 1;
            }
            
            archive.extract(outputDir.string());
        }
        else if (command == "list") {
            auto files = archive.getFileList();
            std::cout << "Archive contents (" << files.size() << " files):\n";
            std::cout << std::string(60, '-') << "\n";
            std::cout << "Name                                              Size      Compressed\n";
            std::cout << std::string(60, '-') << "\n";
            
            for (const auto& file : files) {
                // Fixed: Access the name member and print formatted output
                printf("%-48s %10llu %10llu\n", 
                       file.name.c_str(), 
                       static_cast<unsigned long long>(file.originalSize),
                       static_cast<unsigned long long>(file.compressedSize));
            }
        }
        else {
            std::cerr << "Unknown command: " << command << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}