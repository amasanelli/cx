CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized -nostdlib

.PHONY: clean all

all: clean bin/sleep_inline bin/sleep_asm

bin:
	mkdir -p bin

bin/sleep_inline: main_inline.c | bin
	$(CC) $(CFLAGS) -o $@ $^

bin/syscall.o: syscall.asm | bin
	$(CC) -c -x assembler $< -o $@

bin/sleep_asm: main_asm.c bin/syscall.o | bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) bin/sleep_inline bin/sleep_asm bin/syscall.o
