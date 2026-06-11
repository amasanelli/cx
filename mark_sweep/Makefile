CC=gcc
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -I ./
C_FILES=object.c stack.c vm.c

.PHONY: all clean test

all: clean mark_sweep

test: all
	./mark_sweep

mark_sweep: main.c $(C_FILES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) mark_sweep
