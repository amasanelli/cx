CC=gcc
FLAGS=-Wall -Werror -O3 -std=gnu89 -pedantic-errors -I ./
C_FILES=object.c set.c
H_FILES=object.h set.h

all: clean refcount

refcount: main.c $(C_FILES) $(H_FILES)
	$(CC) $(FLAGS) -o refcount main.c $(C_FILES)

clean:
	rm -f refcount