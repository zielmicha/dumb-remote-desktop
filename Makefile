CC=gcc
CFLAGS=-g -std=gnu99
SERVER_LDFLAGS=-g -lX11
CLIENT_LDFLAGS=-g -static

all: x11_server client

x11_support.o: x11_support.c x11_support.h
	$(CC) $(CFLAGS) -c x11_support.c -o $@ -Wno-implicit-function-declaration
server.o: server.c x11_support.h protocol.h
	$(CC) $(CFLAGS) -c server.c -o $@
protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -c protocol.c -o $@
x11_server: x11_support.o server.o protocol.o
	$(CC) $(SERVER_LDFLAGS) $^ -o $@
client.o: client.c protocol.h
	$(CC) $(CFLAGS) -c client.c -o $@
client: client.o protocol.o
	$(CC) $(CLIENT_LDFLAGS) $^ -o $@
