#include "ArchiveConsole.h"
#include <iostream>
#include <filesystem>
#include <set>
#include <cstring>

void ArchiveConsole::printUsage() const {
    std::cout << "Usage: archive <command> [options] <arguments>\n\n";
    std::cout << "Commands:\n";
    std::cout << "  create <archive_name> <file1> [file2 ...]  Create a new archive\n";
    std::cout << "  add <archive_name> <file1> [file2 ...]     Add files to existing archive\n";
    std::cout << "  extract <archive_name> [output_dir]        Extract files from an archive\n";
    std::cout << "  list <archive_name>                        List contents of an archive\n";
    std::cout << "  selfext <output.exe> [options] <files...>  Create self-extracting executable\n";
    std::cout << "\nSelf-extracting options:\n";
    std::cout << "  --exec <command>    Command to run after extraction (e.g., msiexec)\n";
    std::cout << "  --args <arguments>  Arguments for the command (e.g., \"/i app.msi /quiet\")\n";
    std::cout << "  --silent            Run command without showing window\n";
    std::cout << "  --no-wait           Don't wait for command to complete\n";
    std::cout << "  --workdir <dir>     Working directory for command execution\n";
    std::cout << "  --stub <path>       Use custom extractor stub executable\n";
    std::cout << "\nCompression options:\n";
    std::cout << "  --best              Maximum compression (slower)\n";
    std::cout << "  --fastest           Fastest compression (larger files)\n";
    std::cout << "  --normal            Balanced compression (default)\n";
}

bool ArchiveConsole::createArchive(const std::string& archiveName, int argc, char* argv[]) {
    progress.startTracking("Creating archive");
    Archive archive(archiveName);
    std::vector<std::filesystem::path> files;
    std::set<std::filesystem::path> uniqueFiles;
    // Arguments after archiveName are files/dirs
    for (int i = 3; i < argc; ++i) {
        std::filesystem::path inputPath(argv[i]);
        if (std::filesystem::is_directory(inputPath)) {
            for (auto& entry : std::filesystem::recursive_directory_iterator(inputPath)) {
                if (std::filesystem::is_regular_file(entry)) {
                    uniqueFiles.insert(entry.path());
                }
            }
        } else if (std::filesystem::is_regular_file(inputPath)) {
            uniqueFiles.insert(inputPath);
        } else {
            std::cerr << "Warning: Skipping non-existent or unsupported path: " << inputPath << std::endl;
        }
    }
    files.assign(uniqueFiles.begin(), uniqueFiles.end());
    if (files.empty()) {
        std::cerr << "Error: No input files found to archive.\n";
        progress.finishTracking();
        return false;
    }
    archive.create(files);
    progress.finishTracking();
    return true;
}

bool ArchiveConsole::extractArchive(const std::string& archiveName, const std::string& outputDir) {
    progress.startTracking("Extracting archive");
    Archive archive(archiveName);
    archive.extract(outputDir);
    progress.finishTracking();
    return true;
}

bool ArchiveConsole::listArchiveContents(const std::string& archiveName) const {
    Archive archive(archiveName);
    const auto entries = archive.getFileList();
    std::cout << "Contents of '" << archiveName << "':" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "Name                                              Size      Compressed" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const auto& entry : entries) {
        printf("%-48s %10llu %10llu\n",
               entry.name.c_str(),
               static_cast<unsigned long long>(entry.originalSize),
               static_cast<unsigned long long>(entry.compressedSize));
    }
    return true;
}

bool ArchiveConsole::addToArchive(const std::string& archiveName, int argc, char* argv[]) {
    if (!std::filesystem::exists(archiveName)) {
        std::cerr << "Error: Archive does not exist: " << archiveName << std::endl;
        return false;
    }

    progress.startTracking("Adding to archive");
    Archive archive(archiveName);
    std::vector<std::filesystem::path> files;
    std::set<std::filesystem::path> uniqueFiles;

    // Arguments after archiveName are files/dirs (starting at index 3)
    for (int i = 3; i < argc; ++i) {
        std::filesystem::path inputPath(argv[i]);
        if (std::filesystem::is_directory(inputPath)) {
            for (auto& entry : std::filesystem::recursive_directory_iterator(inputPath)) {
                if (std::filesystem::is_regular_file(entry)) {
                    uniqueFiles.insert(entry.path());
                }
            }
        } else if (std::filesystem::is_regular_file(inputPath)) {
            uniqueFiles.insert(inputPath);
        } else {
            std::cerr << "Warning: Skipping non-existent or unsupported path: " << inputPath << std::endl;
        }
    }

    files.assign(uniqueFiles.begin(), uniqueFiles.end());
    if (files.empty()) {
        std::cerr << "Error: No files found to add.\n";
        progress.finishTracking();
        return false;
    }

    archive.add(files, compressionType);

    if (verboseOutput) {
        std::cout << "Added " << files.size() << " file(s) to " << archiveName << std::endl;
    }

    progress.finishTracking();
    return true;
}

int ArchiveConsole::parseSelfExtractOptions(int argc, char* argv[],
                                            AutoExecConfig& config,
                                            std::vector<std::filesystem::path>& files,
                                            std::string& stubPath) {
    int i = 3;  // Start after "selfext" and output path

    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--exec" && i + 1 < argc) {
            config.command = argv[++i];
        } else if (arg == "--args" && i + 1 < argc) {
            config.arguments = argv[++i];
        } else if (arg == "--silent") {
            config.silent = true;
        } else if (arg == "--no-wait") {
            config.waitForCompletion = false;
        } else if (arg == "--workdir" && i + 1 < argc) {
            config.workingDir = argv[++i];
        } else if (arg == "--stub" && i + 1 < argc) {
            stubPath = argv[++i];
        } else if (arg == "--best") {
            compressionType = CompressionType::Best;
        } else if (arg == "--fastest") {
            compressionType = CompressionType::Fastest;
        } else if (arg == "--normal") {
            compressionType = CompressionType::Normal;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            return -1;
        } else {
            // Non-option argument - this is a file
            std::filesystem::path inputPath(arg);
            if (std::filesystem::is_directory(inputPath)) {
                for (auto& entry : std::filesystem::recursive_directory_iterator(inputPath)) {
                    if (std::filesystem::is_regular_file(entry)) {
                        files.push_back(entry.path());
                    }
                }
            } else if (std::filesystem::is_regular_file(inputPath)) {
                files.push_back(inputPath);
            } else {
                std::cerr << "Warning: Skipping non-existent path: " << inputPath << std::endl;
            }
        }
        ++i;
    }

    return 0;
}

bool ArchiveConsole::createSelfExtracting(const std::string& outputPath, int argc, char* argv[]) {
    progress.startTracking("Creating self-extracting archive");

    AutoExecConfig config;
    std::vector<std::filesystem::path> files;
    std::string stubPath;

    if (parseSelfExtractOptions(argc, argv, config, files, stubPath) < 0) {
        progress.finishTracking();
        return false;
    }

    if (files.empty()) {
        std::cerr << "Error: No files specified for self-extracting archive.\n";
        progress.finishTracking();
        return false;
    }

    if (verboseOutput) {
        std::cout << "Creating self-extracting archive: " << outputPath << std::endl;
        std::cout << "Files to include: " << files.size() << std::endl;
        if (!config.command.empty()) {
            std::cout << "Auto-execute: " << config.command;
            if (!config.arguments.empty()) {
                std::cout << " " << config.arguments;
            }
            std::cout << std::endl;
        }
    }

    try {
        // Create a temporary archive name based on output path
        std::string tempArchive = outputPath + ".tmp.arc";
        Archive archive(tempArchive);
        archive.createSelfExtracting(files, outputPath, compressionType, config, stubPath);

        // Clean up temporary archive if it exists
        if (std::filesystem::exists(tempArchive)) {
            std::filesystem::remove(tempArchive);
        }

        if (verboseOutput) {
            auto size = std::filesystem::file_size(outputPath);
            std::cout << "Created: " << outputPath << " (" << size << " bytes)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating self-extracting archive: " << e.what() << std::endl;
        progress.finishTracking();
        return false;
    }

    progress.finishTracking();
    return true;
}