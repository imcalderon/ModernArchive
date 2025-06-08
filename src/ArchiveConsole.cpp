#include "ArchiveConsole.h"
#include <iostream>
#include <filesystem>

void ArchiveConsole::printUsage() const {
    std::cout << "Usage: archive <command> <options>\n";
    std::cout << "Commands:\n";
    std::cout << "  create <archive_name> <file1> [file2 ...]  Create a new archive\n";
    std::cout << "  extract <archive_name> [output_dir]        Extract files from an archive\n";
    std::cout << "  list <archive_name>                        List contents of an archive\n";
}

bool ArchiveConsole::createArchive(const std::string& archiveName) {
    progress.startTracking("Creating archive");
    Archive archive(archiveName);
    std::vector<std::string> files; // TODO: Implement file list collection
    bool result = archive.createArchive(files);
    progress.finishTracking();
    return result;
}

bool ArchiveConsole::extractArchive(const std::string& archiveName, const std::string& outputDir) {
    progress.startTracking("Extracting archive");
    Archive archive(archiveName);
    bool result = archive.extractArchive(outputDir);
    progress.finishTracking();
    return result;
}

bool ArchiveConsole::listArchiveContents(const std::string& archiveName) const {
    Archive archive(archiveName);
    const auto& entries = archive.getEntries();
    std::cout << "Contents of '" << archiveName << "':" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Name                                    Size      Compressed" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    for (const auto& entry : entries) {
        printf("%-40s %8llu %8llu\n", 
               entry.name.c_str(), 
               entry.originalSize,
               entry.compressedSize);
    }
    return true;
}