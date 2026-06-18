CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized -nostdlib

.PHONY: clean all

all: clean bin/sleep_c_inline bin/sleep_c_extern bin/sleep_pure

bin:
	mkdir -p bin

bin/sleep_c_inline: sleep_c_inline.c | bin
	$(CC) $(CFLAGS) -o $@ $^

sleep_c_extern.o: sleep_c_extern.asm | bin
	$(CC) -c -x assembler $< -o $@

bin/sleep_c_extern: sleep_c_extern.c sleep_c_extern.o | bin
	$(CC) $(CFLAGS) -o $@ $^

bin/sleep_pure: sleep_pure.asm | bin
	$(CC) -nostdlib -x assembler $< -o $@

clean:
	$(RM) bin/sleep_c_inline bin/sleep_c_extern bin/sleep_pure sleep_c_extern.o
