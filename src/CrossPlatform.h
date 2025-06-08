#ifndef CROSSPLATFORM_H
#define CROSSPLATFORM_H

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

class CrossPlatform {
public:
    static void sleep(int milliseconds);
    static void clearConsole();
};

#endif // CROSSPLATFORM_H