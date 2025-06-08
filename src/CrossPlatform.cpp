// filepath: e:\proj\archiver\src\CrossPlatform.cpp
#include "CrossPlatform.h"

// Platform-specific implementations for cross-platform compatibility

#ifdef _WIN32
#include <windows.h>
#include <iostream>

void clearConsole() {
    system("cls");
}

void sleepFor(int milliseconds) {
    Sleep(milliseconds);
}

#else
#include <unistd.h>
#include <iostream>

void clearConsole() {
    system("clear");
}

void sleepFor(int milliseconds) {
    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

#endif

void printPlatformInfo() {
#ifdef _WIN32
    std::cout << "Running on Windows" << std::endl;
#elif defined(__APPLE__)
    std::cout << "Running on macOS" << std::endl;
#else
    std::cout << "Running on Linux/Unix" << std::endl;
#endif
}