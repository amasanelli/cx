CC=gcc
RM=rm -f
CFLAGS=-Wall -Wsign-compare -Werror -O3 -std=gnu89 -pedantic-errors -Wuninitialized -Wmaybe-uninitialized
SRC=icmp.c socket.c net.c ip.c eth.c functions.c arp.c

.PHONY: clean all ns-setup ns-clean ns-ping ns-pong

all: clean bin/ping bin/pong

bin:
	mkdir -p bin

bin/ping: client_main.c $(SRC) | bin
	$(CC) $(CFLAGS) -o $@ $^

bin/pong: server_main.c $(SRC) | bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) bin/ping bin/pong

ns-setup:
	sudo bash ns.sh setup

ns-clean:
	sudo bash ns.sh clean

ns-ping: bin/ping
	sudo bash ns.sh ping

ns-pong: bin/pong
	sudo bash ns.sh pong