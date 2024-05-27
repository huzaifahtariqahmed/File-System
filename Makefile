CC=gcc
CFLAGS=-Wall

all: file_system

file_system: file_system.c
	$(CC) $(CFLAGS) -o file_system file_system.c

run: file_system
	./file_system sampleinput.txt