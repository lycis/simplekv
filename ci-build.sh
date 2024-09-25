#!/bin/bash
mkdir dist


# Run server tests
echo "⚙️ Building server unit tests..."
gcc -fdiagnostics-color=always -DWIN64 -DUNIT_TEST -g server.c -o dist/server_test.exe -lws2_32
if [ $? -ne 0 ]; then
    echo "❌ Compilation of server unit tests failed"
    exit 1
fi
echo "✅ Compilation of server unit tests successful"

echo "⚙️ Running server unit tests..."
./dist/server_test.exe
if [ $? -ne 0 ]; then
    echo "❌ Server unit tests failed"
    exit 1
fi
echo "✅ Server unit tests passed."


# Compile the server
echo "⚙️ Building server.exe for win64 ..."
gcc -fdiagnostics-color=always -DWIN64 -g server.c -o dist/server_x64.exe -lws2_32
if [ $? -ne 0 ]; then
    echo "❌ Compilation of server.exe failed"
    exit 1
fi
echo "✅ Compilation of server.exe for win64 successful"

# Compile the client
echo "⚙️ Building client.exe for win64 ..."
gcc -fdiagnostics-color=always -g client.c -o dist/client_x64.exe -lws2_32
if [ $? -ne 0 ]; then
    echo "❌ Compilation of client.exe failed"
    exit 1
fi
echo "✅ Compilation of client.exe for win64 successful"

echo "✅ All builds successful!"
