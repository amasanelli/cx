CC=gcc
FLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -I ./
C_FILES=stack.c
H_FILES=stack.h

all: clean stack

stack: main.c $(C_FILES) $(H_FILES)
	$(CC) $(FLAGS) -o stack main.c $(C_FILES)

clean:
	rm -f stack