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
    std::vector<std::string> files = {testFile.string()};
    EXPECT_TRUE(archive->createArchive(files));
    EXPECT_TRUE(fs::exists(testArchiveName));
}

TEST_F(ArchiveTest, TestAddFileToArchive) {
    std::vector<std::string> files;
    ASSERT_TRUE(archive->createArchive(files));
    EXPECT_TRUE(archive->addFile(exampleFile.string()));
    // Verify file was added by checking archive size
    EXPECT_GT(fs::file_size(testArchiveName), 0);
}

TEST_F(ArchiveTest, TestExtractArchive) {
    std::vector<std::string> files = {testFile.string()};
    ASSERT_TRUE(archive->createArchive(files));
    EXPECT_TRUE(archive->extractArchive(outputDir.string()));
    EXPECT_TRUE(fs::exists(outputDir / testFile.filename()));
}

TEST_F(ArchiveTest, TestProgressTracking) {
    std::vector<std::string> files = {exampleFile.string()};
    archive->createArchive(files);
    
    progress.startTracking("Testing progress");
    EXPECT_EQ(progress.getProgress(), 0); // Initially 0%
    
    progress.updateProgress(50);
    EXPECT_EQ(progress.getProgress(), 50); // Updated to 50%
    
    progress.finishTracking();
    EXPECT_EQ(progress.getProgress(), 100); // Finished at 100%
}

TEST_F(ArchiveTest, TestInvalidArchive) {
    EXPECT_FALSE(archive->extractArchive("nonexistent_directory"));
    EXPECT_FALSE(archive->addFile("nonexistent_file.txt"));
}