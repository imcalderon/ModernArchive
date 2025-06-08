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
    std::vector<std::filesystem::path> files; // TODO: Implement file list collection
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