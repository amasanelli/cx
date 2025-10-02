CC=gcc
FLAGS=-Wall -Werror -O3 -std=gnu89 -pedantic-errors -I ./
C_FILES=object.c
H_FILES=object.h

all: clean object

object: main.c $(C_FILES) $(H_FILES)
	$(CC) $(FLAGS) -o object main.c $(C_FILES)

clean:
	rm -f object