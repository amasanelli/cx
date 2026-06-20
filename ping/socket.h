#ifndef SOCKET_H
#define SOCKET_H

#include <string.h>     /* memset */
#include <sys/socket.h> /* socket, sendto, recvfrom, struct sockaddr, ssize_t, AF_PACKET, SOCK_RAW */
#include <unistd.h>     /* close */
#include <net/if.h>     /* if_nametoindex */
#include "types.h"      /* u8, u16, u32 */
#include "net.h"        /* write_be16, write_be32 */
#include "eth.h"

#define ETH_P_ALL 0x0003
/* #define SOCK_RAW 3 */
/* #define AF_PACKET 17 */

/* maps to sockaddr_ll */
typedef struct __attribute__((packed))
{
  u16 family;   /* address family (AF_*) */
  u16 protocol; /* ethertype, network byte order */
  int ifindex;  /* interface index */
  u16 hatype;   /* hardware address type (ARPHRD_*) */
  u8 pkttype;   /* packet type (PACKET_HOST, PACKET_OUTGOING, ...) */
  u8 halen;     /* hardware address length */
  u8 addr[8];   /* hardware address */
} skt_addr;

int open_raw_eth_socket(int *skt);

int send_packet(int skt, const skt_addr *addr, const u8 *pkt, u32 pkt_len);

int build_socket_address(int ifindex, skt_addr *addr);

int receive_packet(int skt, u8 *buf, u32 buff_len, u32 *rec);

int get_ifindex(const char *iface, int *ifindex);

#endif
