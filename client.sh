#!/bin/bash
set -e
gcc -o client client.cpp crc.cpp -lstdc++
./client 127.0.0.1 1234 device1 1 foo
./client 127.0.0.1 1234 device1 2 foo
./client 127.0.0.1 1234 device1 3 foo
./client 127.0.0.1 1234 device3 5 foo
./client 127.0.0.1 1234 device2 0 foo
./client 127.0.0.1 1234 device2 1 foo
./client 127.0.0.1 1234 device3 2 foo # this message will be ignored
./client 127.0.0.1 1234 device2 2 foo
./client 127.0.0.1 1234 device2 2 foo # this message will be ignored
./client 127.0.0.1 1234 device2 3 foo
./client 127.0.0.1 1234 device2 4 bar # invalid password
./client 127.0.0.1 1234 device2 5 foo