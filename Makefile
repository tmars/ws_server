# компилятор, используемый для сборки
CC=gcc
# флаги, которые передаются компилятору
CFLAGS=-w -std=c99
# директории
SRC=src/
BIN=bin/

all: server.o base64.o sha1.o client.o websocket.o http.o
	$(CC) $(CFLAGS) $(BIN)server.o $(BIN)base64.o $(BIN)sha1.o $(BIN)client.o $(BIN)websocket.o $(BIN)http.o -o $(BIN)server

server.o:
	$(CC) $(CFLAGS) $(SRC)server.c -c -o $(BIN)server.o

base64.o:
	$(CC) $(CFLAGS) $(SRC)base64.c -c -o $(BIN)base64.o

sha1.o:
	$(CC) $(CFLAGS) $(SRC)sha1.c -c -o $(BIN)sha1.o

client.o:
	$(CC) $(CFLAGS) $(SRC)client.c -c -o $(BIN)client.o

websocket.o:
	$(CC) $(CFLAGS) $(SRC)websocket.c -c -o $(BIN)websocket.o

http.o:
	$(CC) $(CFLAGS) $(SRC)http.c -c -o $(BIN)http.o

run:
	$(BIN)server

clean:
	rm -rf $(BIN)*