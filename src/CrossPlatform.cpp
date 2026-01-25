// filepath: e:\proj\ModernArchive\src\CrossPlatform.cpp
#include "CrossPlatform.h"
#include <iostream>

// Platform-specific implementations for cross-platform compatibility

#ifdef _WIN32
#include <windows.h>

void CrossPlatform::sleep(int milliseconds) {
    Sleep(milliseconds);
}

void CrossPlatform::clearConsole() {
    system("cls");
}

#else
#include <unistd.h>

void CrossPlatform::sleep(int milliseconds) {
    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

void CrossPlatform::clearConsole() {
    system("clear");
}

#endif