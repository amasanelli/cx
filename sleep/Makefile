CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized -nostdlib

.PHONY: clean all

all: clean bin/sleep_c_inline bin/sleep_c_extern bin/sleep_pure

bin:
	mkdir -p bin

# '| bin' = order-only prereq: create dir if missing, but dir timestamp never triggers rebuild
#   target: deps | order-only-deps
#   left of |  -> normal prereqs: timestamp change = rebuild
#   right of | -> only existence matters, not timestamp
bin/sleep_c_inline: sleep_c_inline.c | bin
	$(CC) $(CFLAGS) -o $@ $^

sleep_c_extern.o: sleep_c_extern.asm | bin
	$(CC) -c -x assembler $< -o $@

bin/sleep_c_extern: sleep_c_extern.c sleep_c_extern.o | bin
	$(CC) $(CFLAGS) -o $@ $^

bin/sleep_pure: sleep_pure.asm | bin
	$(CC) -nostdlib -x assembler $< -o $@

asm: all
	@for bin in bin/sleep_c_inline bin/sleep_c_extern bin/sleep_pure; do \
		objdump -d -M intel $$bin > $$bin.asm; \
	done

clean:
	$(RM) bin/sleep_c_inline bin/sleep_c_extern bin/sleep_pure sleep_c_extern.o
