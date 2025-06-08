// filepath: e:\proj\archiver\src\ArchiveProgress.cpp

#include "ArchiveProgress.h"

ArchiveProgress::ArchiveProgress() : currentProgress(0) {
}

void ArchiveProgress::startTracking(const std::string& operation) {
    currentOperation = operation;
    currentProgress = 0;
}

void ArchiveProgress::updateProgress(int percentage) {
    if (percentage >= 0 && percentage <= 100) {
        currentProgress = percentage;
    }
}

void ArchiveProgress::finishTracking() {
    currentProgress = 100;
    currentOperation.clear();
}

int ArchiveProgress::getProgress() const {
    return currentProgress;
}

void ArchiveProgress::reset() {
    currentProgress = 0;
    currentOperation.clear();
}