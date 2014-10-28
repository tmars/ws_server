# компилятор, используемый для сборки
CC=gcc
# флаги, которые передаются компилятору
CFLAGS=-Wall -std=c99
# директории
SRC=src/
BIN=bin/

all:
	$(CC) $(CFLAGS) $(SRC)server.c -o $(BIN)server

run:
	$(BIN)server

clean:
	rm -rf $(BIN)*