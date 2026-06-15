#ifndef SOCKET_H
#define SOCKET_H

#include "types.h" /* u8, u32 */

typedef struct __attribute__((packed))
{
  u16 family;
  u16 port;
  u8  ip[4];
  u8  pad[8]; /* pad to sockaddr_in size */
} skt_addr;

int open_raw_icmp_socket(int *skt);

int open_raw_ip_socket(int *skt);

int send_packet(int skt, const u8 *addr, u32 addr_len, const u8 *pkt, u32 pkt_len);

int build_socket_address(u32 ip, u16 port, u8 **addr, u32 *addr_len);

#endif
