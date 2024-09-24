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
