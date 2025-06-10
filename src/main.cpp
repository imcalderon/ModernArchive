#include <iostream>
#include <string>
#include "ArchiveConsole.h"

int main(int argc, char* argv[]) {
    ArchiveConsole console;

    if (argc < 2) {
        console.printUsage();
        return 1;
    }

    std::string command = argv[1];

    try {
        if (command == "create") {
            if (argc < 4) {
                std::cerr << "Error: Please provide archive name and at least one file.\n";
                console.printUsage();
                return 1;
            }
            std::string archiveName = argv[2];
            if (!console.createArchive(archiveName, argc, argv)) {
                std::cerr << "Error: Failed to create archive.\n";
                return 1;
            }
        }
        else if (command == "extract") {
            if (argc < 3) {
                std::cerr << "Error: Please provide archive name.\n";
                console.printUsage();
                return 1;
            }
            std::string archiveName = argv[2];
            std::string outputDir = (argc >= 4) ? argv[3] : ".";
            if (!console.extractArchive(archiveName, outputDir)) {
                std::cerr << "Error: Failed to extract archive.\n";
                return 1;
            }
        }
        else if (command == "list") {
            if (argc < 3) {
                std::cerr << "Error: Please provide archive name.\n";
                console.printUsage();
                return 1;
            }
            std::string archiveName = argv[2];
            if (!console.listArchiveContents(archiveName)) {
                std::cerr << "Error: Failed to list archive contents.\n";
                return 1;
            }
        }
        else {
            std::cerr << "Error: Unknown command '" << command << "'.\n";
            console.printUsage();
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}