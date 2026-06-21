CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized
SRC=icmp.c socket.c net.c ip.c eth.c functions.c arp.c

.PHONY: clean all

all: clean bin/ping 

bin: 
	mkdir -p bin

bin/ping: main.c $(SRC) | bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) bin/ping