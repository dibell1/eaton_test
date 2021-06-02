#!/bin/bash
set -e
printf "Server compilation...\n"
gcc -o server server.cpp crc.cpp -lstdc++
printf "Waiting for devices...\n"
./server 1234 foo