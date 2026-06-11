CC=gcc
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -I ./
C_FILES=object.c 

.PHONY: all clean

all: clean object

object: main.c $(C_FILES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) object
