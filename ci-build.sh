#!/bin/bash
mkdir dist

# Compile the server
gcc -fdiagnostics-color=always -g server.c -o dist/server.exe -lws2_32
if [ $? -ne 0 ]; then
    echo "Compilation of server.c failed"
    exit 1
fi

# Compile the client
gcc -fdiagnostics-color=always -g client.c -o dist/client.exe -lws2_32
if [ $? -ne 0 ]; then
    echo "Compilation of client.c failed"
    exit 1
fi

echo "Compilation successful!"

# Run server tests
gcc -fdiagnostics-color=always -DWIN64 -DUNIT_TEST -g server.c -o dist/server_test.exe -lws2_32
if [ $? -ne 0 ]; then
    echo "Compilation of server unit tests failed"
    exit 1
fi

./dist/server_test.exe
if [ $? -ne 0 ]; then
    echo "Server unit tests failed"
    exit 1
fi
