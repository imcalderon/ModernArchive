#include <gtest/gtest.h>
#include "Archive.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <sstream>

namespace fs = std::filesystem;

class CompressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        tempDir = fs::temp_directory_path() / "archive_compression_test";
        fs::create_directories(tempDir);
        
        // Create test content (make it larger to see compression benefits)
        std::stringstream content;
        for (int i = 0; i < 100; ++i) {
            content << "This is test content for compression. Line " << i << "\n";
            content << "It includes multiple repetitive lines of text to ensure compression is effective.\n";
        }
        testContent = content.str();
        
        // Create a test file with content
        testFilePath = tempDir / "test.txt";
        std::ofstream testFile(testFilePath);
        testFile << testContent;
        testFile.close();
        
        // Set up directories
        archivePath = tempDir / "test.arc";
        extractDir = tempDir / "extracted";
        fs::create_directories(extractDir);
        
        // Create archive instance
        archive = std::make_unique<Archive>(archivePath.string());
    }

    void TearDown() override {
        try {
            fs::remove_all(tempDir);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to clean up test files: " << e.what() << std::endl;
        }
    }

    std::string readFileContents(const fs::path& path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    fs::path tempDir;
    fs::path testFilePath;
    fs::path archivePath;
    fs::path extractDir;
    std::string testContent;
    std::unique_ptr<Archive> archive;
};

TEST_F(CompressionTest, CompressAndDecompress) {
    // Test file creation and compression
    std::vector<std::string> files = {testFilePath.string()};
    EXPECT_TRUE(archive->createArchive(files));
    
    // Verify archive was created
    EXPECT_TRUE(fs::exists(archivePath));
    
    // Extract the archive
    EXPECT_TRUE(archive->extractArchive(extractDir.string()));
    
    // Verify extracted file exists
    fs::path extractedFile = extractDir / testFilePath.filename();
    EXPECT_TRUE(fs::exists(extractedFile));
    
    // Verify compressed file is smaller (should work now with larger test data)
    auto originalSize = fs::file_size(testFilePath);
    auto compressedSize = fs::file_size(archivePath);
    EXPECT_LT(compressedSize, originalSize) << "Compressed file should be smaller than original";
    
    // Read and compare content
    std::string extractedContent = readFileContents(extractedFile);
    EXPECT_EQ(testContent, extractedContent);
}
