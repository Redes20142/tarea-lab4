# environment variables. Compiler and its flags
# to build a debuggable version call make as: "make G=-g"
# this also can be used to pass custom flags to gcc to build the progam
# be aware that the G flags will also be present on executable builds, so be wise when using it.
CC = gcc
CFLAGS = -c -Wall -x c -I ./include/ -O2 -ffunction-sections -fdata-sections -funit-at-a-time -std=c99 -D_GNU_SOURCE

all : build

build : main.o
	$(CC) $(G) lib/main.o -o bin/tarea-lab4 # TODO add tags for all necesary libs to make the main
	chmod 774 bin/tarea-lab4

main.o :
	$(CC) $(CFLAGS) $(G) src/main.c -o lib/main.o
# TODO insert makefile tasks

client : http_client.o
	$(CC) $(G) lib/http_client.o -o bin/client

http_client.o :
	$(CC) $(CFLAGS) $(G) src/http_client.c -o lib/http_client.o

clean :
	rm -f bin/*

cleanall :
	rm -f bin/* lib/*

exec :
	bin/tarea-lab4

