CC = gcc
CFLAGS = -Wall -Wextra -g -std=c17

.PHONY: shell coreutils

all: shell coreutils

shell: shell/*.c
	$(CC) $(CFLAGS) shell/*.c -o myshell

coreutils: bin/
	for f in coreutils/*.c; do $(CC) $(CFLAGS) "$$f" -o "bin/$$(basename "$$f" .c)"; done

test: shell
	for t in tests/*/*.sh; do $$t; done

bin/:
	mkdir bin

clean:
	rm -f myshell
	rm -rf bin
	rm -rf ./tests/*/out/*
	rm -rf ./tests/*/exp/*
