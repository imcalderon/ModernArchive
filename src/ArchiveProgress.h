#ifndef ARCHIVE_PROGRESS_H
#define ARCHIVE_PROGRESS_H

#include <string>

class ArchiveProgress {
public:
    ArchiveProgress();
    
    void startTracking(const std::string& operation);
    void updateProgress(int percentage);
    void finishTracking();
    int getProgress() const;
    void reset();

private:
    std::string currentOperation;
    int currentProgress;
};

#endif // ARCHIVE_PROGRESS_H