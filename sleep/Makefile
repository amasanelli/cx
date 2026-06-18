CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized -nostdlib

.PHONY: clean all

all: clean bin/sleep_inline bin/sleep_asm bin/sleep_min

bin:
	mkdir -p bin

bin/sleep_inline: main_inline.c | bin
	$(CC) $(CFLAGS) -o $@ $^

syscall.o: syscall.asm | bin
	$(CC) -c -x assembler $< -o $@

bin/sleep_asm: main_asm.c syscall.o | bin
	$(CC) $(CFLAGS) -o $@ $^

bin/sleep_min: sleep_min.asm | bin
	$(CC) -nostdlib -x assembler $< -o $@

clean:
	$(RM) bin/sleep_inline bin/sleep_asm bin/sleep_min syscall.o
