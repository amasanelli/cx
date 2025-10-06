CC=gcc
CFLAGS=-Wall -Werror -O3 -std=gnu89 -pedantic-errors -I ./
SRC=object_new.c object.c stack.c vm.c
OBJ=$(SRC:.c=.o)

.PHONY: clean  

all: mark_sweep 

mark_sweep: main.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o mark_sweep
 