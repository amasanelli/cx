CC=gcc
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -I ./
C_FILES=stack.c

.PHONY: all clean test

all: clean stack

test: all
	./stack

stack: main.c $(C_FILES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) stack
