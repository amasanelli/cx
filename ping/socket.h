#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>            /* fopen, fgets, sscanf, fclose */
#include <string.h>           /* memset, memcpy, strncpy */
#include <sys/socket.h>       /* socket, sendto, recvfrom, setsockopt, struct sockaddr, ssize_t, AF_PACKET, SOCK_RAW, SO_RCVTIMEO, SO_SNDTIMEO */
#include <sys/time.h>         /* struct timeval */
#include <sys/ioctl.h>        /* ioctl, SIOCGIFINDEX, SIOCGIFHWADDR */
#include <ifaddrs.h>          /* getifaddrs, freeifaddrs */
#include <netpacket/packet.h> /* struct sockaddr_ll */
#include <netinet/in.h>       /* struct sockaddr_in */
#include <unistd.h>           /* close */
#include <net/if.h>           /* struct ifreq, IFNAMSIZ */
#include "types.h"            /* u8, u16, u32 */
#include "net.h"              /* write_be16, read_be32 */
#include "eth.h"              /* ETH_ADDR_LEN */
#include "ip.h"               /* IP_ADDR_LEN */

#define ETH_P_ALL 0x0003   /* capture all Ethernet frame types */
#define RTF_UP 0x0001      /* route is up */
#define RTF_GATEWAY 0x0002 /* destination is a gateway */
/* #define SOCK_RAW 3 */
/* #define AF_PACKET 17 */

/* maps to sockaddr_ll */
typedef struct __attribute__((packed))
{
  u16 family;   /* address family (AF_*) */
  u16 protocol; /* ethertype, network byte order */
  u32 if_i;     /* interface index */
  u16 hatype;   /* hardware address type (ARPHRD_*) */
  u8 pkttype;   /* packet type (PACKET_HOST, PACKET_OUTGOING, ...) */
  u8 halen;     /* hardware address length */
  u8 addr[8];   /* hardware address */
} skt_addr;

int open_raw_eth_socket(int *out_skt);

int send_packet(int skt, const skt_addr *addr, const u8 *pkt, u32 pkt_len);

int build_socket_address(u32 if_i, skt_addr *out_addr);

int receive_packet(int skt, u8 *buf, u32 buff_len, u32 *n_recv);

int set_socket_timeouts(int skt, u32 seconds);

typedef struct
{
  u8 name[IFNAMSIZ];
  u32 index;
  u8 ip[IP_ADDR_LEN];
  u8 mac[ETH_ADDR_LEN];
  u8 netmask[IP_ADDR_LEN];
  u8 gateway[IP_ADDR_LEN];
} iface_info;

int get_iface_info(const u8 *dst_ip, iface_info *out);

#endif
