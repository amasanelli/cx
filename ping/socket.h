#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h> /* IPPROTO_ICMP, IPPROTO_RAW, AF_INET */
#include <netinet/ip.h> /* IP_HDRINCL */
#include <string.h>     /* memset */
#include <sys/socket.h> /* socket, sendto, setsockopt, SOCK_RAW, sockaddr, ssize_t */
#include <unistd.h>     /* close */
#include "types.h"      /* u8, u16, u32 */
#include "net.h"        /* write_be16, write_be32 */

typedef struct __attribute__((packed))
{
  u16 family;
  u8  port[2];
  u8 ip[4];
  u8 pad[8]; /* pad to sockaddr_in size */
} skt_addr;

int open_raw_icmp_socket(int *skt);

int send_packet(int skt, const skt_addr *addr, const u8 *pkt, u32 pkt_len);

int build_socket_address(u32 ip, u16 port, skt_addr *addr);

int receive_packet(int skt, u8 *buf, u32 buff_len, u32 *rec);

#endif
