CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized
SRC=icmp.c socket.c net.c ip.c eth.c functions.c arp.c

.PHONY: clean all ns-setup ns-clean ns-ping

all: clean bin/ping

bin:
	mkdir -p bin

bin/ping: main.c $(SRC) | bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) bin/ping

ns-setup:
	sudo bash ns.sh setup

ns-clean:
	sudo bash ns.sh clean

ns-ping: bin/ping
	sudo bash ns.sh ping