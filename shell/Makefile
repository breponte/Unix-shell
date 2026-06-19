SHELL=/bin/bash
CC = gcc
CFLAGS = -g

all: build run

build:
	gcc main.c -o main

run: main
	./main

test:
	gcc main.c -g -o main
	gdb main

clean:
	rm main
