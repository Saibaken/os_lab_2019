CC=gcc
CCFLAGS=-I.

all : client server

client : libr.o libr.h
	$(CC) -o client libr.o client.c -lpthread $(CCFLAGS)

server : libr.o libr.h
	$(CC) -o server libr.o server.c -lpthread $(CCFLAGS)

libr.o : libr.h
	$(CC) -o libr.o -c libr.c $(CCFLAGS)

clean :
	rm libr.o