CC=gcc
CPP=g++

client: client.c
	$(CC) $< -o $@

server: server.c
	$(CC) $< -o $@
