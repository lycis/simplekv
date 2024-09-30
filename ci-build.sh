#!/bin/bash

# Ensure necessary tools are available
command -v gcc >/dev/null 2>&1 || { echo "❌ gcc is required but not installed. Aborting."; exit 1; }

# Clean up previous build artifacts
echo "⚙️ Cleaning up previous build artifacts..."
rm -rf bin
mkdir -p bin || { echo "❌ Failed to create 'bin' directory"; exit 1; }
echo "✅ Cleanup successful"

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

    echo "⬇️ Downloading DrMemory..."
    wget -O drmemory.zip https://github.com/DynamoRIO/drmemory/releases/download/release_2.6.0/DrMemory-Windows-2.6.0.zip > /dev/null 2>&1
    unzip drmemory.zip -d drmemory > /dev/null 2>&1

    echo "🩺 Running memory leak analysis ($test_file)..."
    ./drmemory/DrMemory-Windows-2.6.0/bin/drmemory -no_follow_children -light -count_leaks -brief -summary -batch -ignore_kernel -- "$test_file" > /dev/null 2>memtest_report.txt
    if grep -q "ERRORS FOUND:" memtest_report.txt; then
        echo "❌ DrMemory detected memory errors"
        echo "------ 📄 DrMemory report ------"
        cat memtest_report.txt
        
        exit 1
    fi
    echo "✅ DrMemory detected no memory errors"
}

# Compile and run server unit tests
compile "Building server unit tests" "src/server.c src/kvstore.c" "bin/server_test.exe" "-DUNIT_TEST -g"
run_tests "bin/server_test.exe"

# Compile server and client simultaneously
compile "Building server.exe for win64" "src/server.c src/kvstore.c" "bin/server_x64.exe" "-g" &
compile "Building client.exe for win64" "src/client.c" "bin/client_x64.exe" "-g" &
wait

echo "✅ All builds successful!"
