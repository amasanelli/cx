CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized

.PHONY: clean all

all: clean bin/sleep

bin:
	mkdir -p bin

bin/sleep: main.c | bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) bin/sleep
