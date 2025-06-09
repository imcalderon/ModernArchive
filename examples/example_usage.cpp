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
              << "  selfext  - Create self-extracting executable: " << programName << " selfext <output.exe> <file1> [file2 ...]\n"
              << "  version  - Show version information\n\n"
              << "Compression Options:\n"
              << "  --fastest  Use fastest compression\n"
              << "  --best    Use best compression\n"
              << "  --normal  Use normal compression (default)\n\n"
              << "Self-Extracting Options:\n"
              << "  --stub <path>      Use custom extractor stub (optional)\n"
              << "  --exec <command>   Command to execute after extraction (e.g., 'msiexec')\n"
              << "  --args <args>      Arguments for the command (e.g., '/i installer.msi /quiet')\n"
              << "  --silent           Run command without showing window\n"
              << "  --no-wait          Don't wait for command completion\n"
              << "  --workdir <dir>    Working directory for command (default: extraction dir)\n\n"
              << "MSI Installer Examples:\n"
              << "  " << programName << " selfext installer.exe --exec msiexec --args \"/i installer.msi /quiet\" installer.msi\n"
              << "  " << programName << " selfext setup.exe --exec msiexec --args \"/i setup.msi\" --silent setup.msi\n";
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

    try {
        if (command == "selfext") {
            // Self-extracting executable creation
            fs::path outputPath = makeAbsolute(argv[2]);
            
            CompressionType compression = CompressionType::Normal;
            std::string stubPath;
            AutoExecConfig autoExec;
            std::vector<fs::path> files;
            
            for (int i = 3; i < argc; i++) {
                std::string arg = argv[i];
                if (arg == "--fastest") {
                    compression = CompressionType::Fastest;
                } else if (arg == "--best") {
                    compression = CompressionType::Best;
                } else if (arg == "--normal") {
                    compression = CompressionType::Normal;
                } else if (arg == "--stub" && i + 1 < argc) {
                    stubPath = argv[++i];
                } else if (arg == "--exec" && i + 1 < argc) {
                    autoExec.command = argv[++i];
                } else if (arg == "--args" && i + 1 < argc) {
                    autoExec.arguments = argv[++i];
                } else if (arg == "--silent") {
                    autoExec.silent = true;
                } else if (arg == "--no-wait") {
                    autoExec.waitForCompletion = false;
                } else if (arg == "--workdir" && i + 1 < argc) {
                    autoExec.workingDir = argv[++i];
                } else {
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

            // Ensure the output directory exists
            if (!ensureDirectoryExists(outputPath.parent_path())) {
                return 1;
            }

            // Sort files to ensure consistent order
            std::sort(files.begin(), files.end());
            
            // Create a temporary archive object for self-extracting
            Archive archive("temp.arc"); // This won't actually be used since we're creating self-extracting
            archive.createSelfExtracting(files, outputPath.string(), compression, autoExec, stubPath);
            
            std::cout << "Self-extracting executable created: " << outputPath << std::endl;
            std::cout << "To extract, run: " << outputPath << " [output_directory]" << std::endl;
            
            if (!autoExec.command.empty()) {
                std::cout << "The executable will automatically run: " << autoExec.command;
                if (!autoExec.arguments.empty()) {
                    std::cout << " " << autoExec.arguments;
                }
                std::cout << std::endl;
            }
        }
        else {
            // Original archive functionality
            fs::path archivePath = makeAbsolute(argv[2]);
            
            // Ensure the archive's parent directory exists for create/add commands
            if (command == "create" || command == "add") {
                if (!ensureDirectoryExists(archivePath.parent_path())) {
                    return 1;
                }
            }

            Archive archive(archivePath.string());

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
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}