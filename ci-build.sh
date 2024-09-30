#!/bin/bash

ZIG_VERSION=0.14.0
ZIG_BUILD_ID=-dev.1694+3b465ebec
BUILD_TOOLS="./build_tools"

# Ensure necessary tools are available
# command -v gcc >/dev/null 2>&1 || { echo "❌ gcc is required but not installed. Aborting."; exit 1; }

# Clean up previous build artifacts
echo "⚙️ Cleaning up previous build artifacts..."
rm -rf zig-out .zig-cache ${BUILD_TOOLS} .zig-cache
echo "✅ Cleanup successful"

echo "⚙️ Creating build tools directory..."
mkdir -p ${BUILD_TOOLS} || { echo "❌ Failed to create '${BUILD_TOOLS}' directory"; exit 1; }

# Function to compile code
compile() {
    local description=$1
    local source_file=$2
    local output_file=$3
    local flags=$4

    echo "⚙️ $description..."
    gcc -fdiagnostics-color=always $flags $source_file -o "$output_file" -lws2_32
    if [ $? -ne 0 ]; then
        echo "❌ $description failed"
        exit 1
    fi
    echo "✅ $description successful"
}

# Function to run tests
run_tests() {
    local test_file=$1

    echo "⚙️ Running tests..."
    ./"$test_file"
    if [ $? -ne 0 ]; then
        echo "❌ Unit tests failed"
        exit 1
    fi
    echo "✅ Unit tests passed."
}

memory_analysis() {
    ${BUILD_TOOLS}/drmemory/DrMemory-Windows-2.6.0/bin/drmemory -no_follow_children -light -count_leaks -brief -summary -batch -ignore_kernel -- "$test_file" > /dev/null 2>memtest_report.txt
    if grep -q "ERRORS FOUND:" memtest_report.txt; then
        echo "❌ DrMemory detected memory errors"
        echo "------ 📄 DrMemory report ------"
        cat memtest_report.txt
        exit 1
    fi
    echo "✅ DrMemory detected no memory errors"
}

download_zig() {
    echo "⬇️ Downloading Zig..."
    wget -O zig.zip https://ziglang.org/builds/zig-windows-x86_64-${ZIG_VERSION}${ZIG_BUILD_ID}.zip > /dev/null 2>&1
    unzip zig.zip > /dev/null 2>&1
    mv zig-windows-x86_64-${ZIG_VERSION}${ZIG_BUILD_ID} ${BUILD_TOOLS}/zig
    if [ $? -ne 0 ]; then
        echo "❌ Failed to move downloade Zig to ${BUILD_TOOLS}/zig"
        exit 1
    fi
    rm zig.zip
    if [ $? -ne 0 ]; then
        echo "⚠️ Failed to remove zig.zip"
    fi
    echo "✅ Zig downloaded successfully"
}

download_drmemory() {
    echo "⬇️ Downloading DrMemory..."
    wget -O drmemory.zip https://github.com/DynamoRIO/drmemory/releases/download/release_2.6.0/DrMemory-Windows-2.6.0.zip > /dev/null 2>&1
    unzip drmemory.zip -d ${BUILD_TOOLS}/drmemory > /dev/null 2>&1
    echo "🩺 Running memory leak analysis ($test_file)..."
    rm drmemory.zip
    if [ $? -ne 0 ]; then
        echo "⚠️ Failed to remove drmemory.zip"
    fi
    echo "✅ DrMemory downloaded successfully"
}

build() {
    echo "⚙️ Building binaries..."
    ${BUILD_TOOLS}/zig/zig build
    if [ $? -ne 0 ]; then
        echo "❌ Build failed"
        exit 1
    fi
    echo "✅ Build successful"
}

# install build tools
download_zig
download_drmemory

build
run_tests "zig-out/bin/server_test.exe"
memory_analysis "zig-out/bin/server_test.exe"

echo "✅ All builds successful!"
