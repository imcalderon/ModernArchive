#include "ArchiveConsole.h"
#include <iostream>
#include <filesystem>
#include <set>

void ArchiveConsole::printUsage() const {
    std::cout << "Usage: archive <command> <options>\n";
    std::cout << "Commands:\n";
    std::cout << "  create <archive_name> <file1> [file2 ...]  Create a new archive\n";
    std::cout << "  extract <archive_name> [output_dir]        Extract files from an archive\n";
    std::cout << "  list <archive_name>                        List contents of an archive\n";
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