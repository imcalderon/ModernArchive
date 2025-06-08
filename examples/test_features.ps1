# Test script for ModernArchive functionality
Write-Host "ModernArchive Test Script" -ForegroundColor Green

# Ensure we're in the right directory and using absolute paths
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
Set-Location $projectRoot

$testDataDir = Join-Path $scriptDir "test_data"
$testOutputDir = Join-Path $scriptDir "test_output"
$exePath = Join-Path $projectRoot "build\Debug\example_usage.exe"

# Ensure the executable exists
if (-not (Test-Path $exePath)) {
    Write-Host "Error: example_usage.exe not found at $exePath" -ForegroundColor Red
    Write-Host "Please build the project first" -ForegroundColor Red
    exit 1
}

# Clean up previous test output and test data
Write-Host "Cleaning up previous test data..." -ForegroundColor Cyan
if (Test-Path $testOutputDir) {
    Remove-Item -Recurse -Force $testOutputDir
}
if (Test-Path $testDataDir) {
    Remove-Item -Recurse -Force $testDataDir
}

# Create test directory structure
Write-Host "Creating test directory structure..." -ForegroundColor Cyan
$testDirs = @(
    "$testDataDir",
    "$testDataDir\subdir1",
    "$testDataDir\subdir1\nested",
    "$testDataDir\subdir2",
    "$testOutputDir"
)

foreach ($dir in $testDirs) {
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
}

# Create text files
$files = @{
    "$testDataDir\sample.txt" = "This is a sample text file for testing ModernArchive`nIt contains multiple lines`nof text that will be compressed.";
    "$testDataDir\sample2.txt" = "This is another sample file`nwith different content`nfor testing multiple file handling.";
    "$testDataDir\subdir1\file1.txt" = "File in subdirectory 1`nwith some content`nthat should compress well`ndue to repetition repetition repetition.";
    "$testDataDir\subdir1\nested\deep.txt" = "This is a deeply nested file`nto test directory structure preservation.";
    "$testDataDir\subdir2\file2.txt" = "Another file in a different subdirectory`nwith unique content.";
}

foreach ($file in $files.GetEnumerator()) {
    $file.Value | Out-File -FilePath $file.Key -Encoding utf8
}

# Create binary files
Write-Host "Creating binary test files..." -ForegroundColor Cyan

# Create a binary file with repeating patterns (should compress well)
$compressibleBytes = New-Object byte[] 1024
for ($i = 0; $i -lt 1024; $i++) {
    $compressibleBytes[$i] = $i % 256
}
[IO.File]::WriteAllBytes("$testDataDir\subdir1\binary1.bin", $compressibleBytes)

# Create a binary file with random data (should compress poorly)
$randomBytes = New-Object byte[] 1024
$rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
$rng.GetBytes($randomBytes)
[IO.File]::WriteAllBytes("$testDataDir\subdir2\binary2.bin", $randomBytes)

# Copy the example_usage.exe as a test binary
Copy-Item $exePath "$testDataDir\subdir1\test.exe"

Write-Host "`nTest data structure created:" -ForegroundColor Green
Get-ChildItem $testDataDir -Recurse | ForEach-Object {
    $indent = "  " * ($_.FullName.Split([IO.Path]::DirectorySeparatorChar).Count - $testDataDir.Split([IO.Path]::DirectorySeparatorChar).Count)
    Write-Host "$indent$($_.Name)"
}

# Function to calculate directory size
function Get-DirectorySize {
    param([string]$Path)
    $size = (Get-ChildItem $Path -Recurse -File | Measure-Object -Property Length -Sum).Sum
    return [math]::Round($size / 1KB, 2)
}

# Test creating self-extracting archive with different compression levels
Write-Host "`nTesting self-extracting archive creation with different compression levels:" -ForegroundColor Cyan

$compressionTypes = @(
    @{name="fast"; option="--fastest"},
    @{name="best"; option="--best"},
    @{name="normal"; option="--normal"}
)

$originalSize = Get-DirectorySize $testDataDir

foreach ($type in $compressionTypes) {
    Write-Host "`n$($type.name) compression:" -ForegroundColor Yellow
    $archivePath = Join-Path $testOutputDir "$($type.name).arc"
    & $exePath create $archivePath $($type.option) "$testDataDir"
    
    $archiveSize = [math]::Round((Get-Item $archivePath).Length / 1KB, 2)
    $ratio = [math]::Round(($archiveSize / $originalSize) * 100, 1)
    
    Write-Host "Original size: ${originalSize}KB"
    Write-Host "Archive size: ${archiveSize}KB"
    Write-Host "Compression ratio: $ratio%"
    
    # List contents
    Write-Host "`n$($type.name) archive contents:" -ForegroundColor Yellow
    & $exePath list $archivePath
    
    # Extract
    $extractPath = Join-Path $testOutputDir "$($type.name)_extracted"
    & $exePath extract $archivePath $extractPath
    
    # Verify structure and files
    Write-Host "`nVerifying $($type.name) extracted files:" -ForegroundColor Yellow
    $originalFiles = Get-ChildItem $testDataDir -Recurse -File
    $extractedFiles = Get-ChildItem $extractPath -Recurse -File
    
    if ($originalFiles.Count -ne $extractedFiles.Count) {
        Write-Host "  Error: File count mismatch. Original: $($originalFiles.Count), Extracted: $($extractedFiles.Count)" -ForegroundColor Red
        continue
    }
    
    $verified = $true
    foreach ($original in $originalFiles) {
        $relativePath = $original.FullName.Substring($testDataDir.Length)
        $extractedPath = Join-Path $extractPath $relativePath
        
        if (-not (Test-Path $extractedPath)) {
            Write-Host "  Error: Missing file $relativePath" -ForegroundColor Red
            $verified = $false
            continue
        }
        
        $originalHash = Get-FileHash $original.FullName
        $extractedHash = Get-FileHash $extractedPath
        
        if ($originalHash.Hash -ne $extractedHash.Hash) {
            Write-Host "  Error: Content mismatch in $relativePath" -ForegroundColor Red
            $verified = $false
        }
    }
    
    if ($verified) {
        Write-Host "  All files and directories verified successfully" -ForegroundColor Green
    }
}
