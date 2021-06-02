# eaton_test

simple UDP server for receiving and counting incoming messages.
compiled by gcc 10.3.0 with stdc++

UDP server waits for incoming message, increases the device counter if password and sequence number are correct otherwise drops the message.
	- password is "encrypted using XOR'
	- seq number prevents from duplication of the message

run server -> ./server.sh
run several clients -> ./client.sh

stop and get statistics

kill -15 $(pgrep server)
