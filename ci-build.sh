#!/bin/bash
mkdir dist
gcc -fdiagnostics-color=always -g server.c -o dist/server.exe -lws2_32
gcc -fdiagnostics-color=always -g client.c -o dist/client.exe -lws2_32