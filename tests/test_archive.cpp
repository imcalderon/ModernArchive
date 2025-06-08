#include <gtest/gtest.h>
#include "Archive.h"
#include "ArchiveProgress.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

class ArchiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directories
        testDir = fs::temp_directory_path() / "archive_test";
        fs::create_directories(testDir);
        outputDir = testDir / "output";
        fs::create_directories(outputDir);

        // Create test files
        testFile = testDir / "test.txt";
        std::ofstream(testFile) << "Test content for archive\n";
        
        exampleFile = testDir / "example.txt";
        std::ofstream(exampleFile) << "Example content for archive\n";

        // Create a test archive
        testArchiveName = (testDir / "test_archive.arc").string();
        archive = std::make_unique<Archive>(testArchiveName);
    }

    void TearDown() override {
        // Clean up test files and directories
        try {
            fs::remove_all(testDir);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to clean up test files: " << e.what() << std::endl;
        }
    }

    std::unique_ptr<Archive> archive;
    std::string testArchiveName;
    ArchiveProgress progress;
    fs::path testDir;
    fs::path outputDir;
    fs::path testFile;
    fs::path exampleFile;
};

TEST_F(ArchiveTest, TestCreateArchive) {
    std::vector<fs::path> files = {testFile};
    archive->create(files);
    EXPECT_TRUE(fs::exists(testArchiveName));
}

TEST_F(ArchiveTest, TestAddFileToArchive) {
    // First create an archive with one file
    std::vector<fs::path> files = {testFile};
    archive->create(files);
    
    // Then add another file
    std::vector<fs::path> newFiles = {exampleFile};
    archive->add(newFiles);
    
    // Re-open the archive to read the updated file list
    auto updatedArchive = std::make_unique<Archive>(testArchiveName);
    auto entries = updatedArchive->getFileList();
    EXPECT_EQ(entries.size(), 2);
}

TEST_F(ArchiveTest, TestExtractArchive) {
    std::vector<fs::path> files = {testFile};
    archive->create(files);
    archive->extract(outputDir.string());
    EXPECT_TRUE(fs::exists(outputDir / testFile.filename()));
}

TEST_F(ArchiveTest, TestProgressTracking) {
    std::vector<fs::path> files = {exampleFile};
    archive->create(files);
    
    progress.startTracking("Testing progress");
    EXPECT_EQ(progress.getProgress(), 0); // Initially 0%
    
    progress.updateProgress(50);
    EXPECT_EQ(progress.getProgress(), 50); // Updated to 50%
    
    progress.finishTracking();
    EXPECT_EQ(progress.getProgress(), 100); // Finished at 100%
}

TEST_F(ArchiveTest, TestInvalidArchive) {
    // Test extracting from non-existent archive
    Archive nonExistentArchive("nonexistent.arc");
    EXPECT_THROW(nonExistentArchive.extract("nonexistent_directory"), std::runtime_error);
    
    // Test adding non-existent file
    std::vector<fs::path> invalidFiles = {"nonexistent_file.txt"};
    EXPECT_THROW(archive->add(invalidFiles), std::runtime_error);
}