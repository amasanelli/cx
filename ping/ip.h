#ifndef IP_H
#define IP_H

#include <stdlib.h> /* malloc, free */
#include <stdio.h>  /* printf */
#include <string.h> /* memset, memcpy */
/* __BYTE_ORDER__ and __ORDER_BIG_ENDIAN__ are GCC built-ins, no include needed */
#include "types.h"  /* u8, u16, u32 */
#include "net.h"    /* write_be16, write_be32, read_be32, checksum */
#include "config.h" /* IP_DEFAULT_TTL */

#define IP_ADDR_LEN 4
#define IP_MAX_PLD_SIZE (65535 - (u32)sizeof(ip_hdr))
#define IP_PROTO_ICMP 1
#define IP_STR_MAX_LEN 16 /* "255.255.255.255\0" */

typedef struct __attribute__((packed))
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  u32 ver : 4; /* IP version */
  u32 ihl : 4; /* header length in 32-bit words */
#else
  u32 ihl : 4; /* header length in 32-bit words */
  u32 ver : 4; /* IP version */
#endif
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  u32 dscp : 6; /* differentiated services code point (traffic priority) */
  u32 ecn : 2;  /* explicit congestion notification */
#else
  u32 ecn : 2;  /* explicit congestion notification */
  u32 dscp : 6; /* differentiated services code point (traffic priority) */
#endif
  u8 tot_len[2]; /* total length: header + payload, network byte order */
  u8 id[2];      /* packet identifier for fragment reassembly, network byte order */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  u32 flags : 3;     /* control bits: DF (don't fragment), MF (more fragments) */
  u32 frag_off : 13; /* fragment offset within the original datagram */
#else
  u32 frag_off : 13; /* fragment offset within the original datagram */
  u32 flags : 3;     /* control bits: DF (don't fragment), MF (more fragments) */
#endif
  u8 ttl;         /* max hops before the packet is discarded */
  u8 protocol;    /* encapsulated protocol (IP_PROTO_ICMP=1, TCP=6, UDP=17) */
  u8 checksum[2]; /* header-only checksum, network byte order */
  u8 src[4];      /* source IP address, network byte order */
  u8 dst[4];      /* destination IP address, network byte order */
} ip_hdr;

int ip_build_packet(u8 protocol, const u8 *src_ip, const u8 *dst_ip, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len);

int build_ip_icmp_packet(const u8 *src_ip, const u8 *dst_ip, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len);

int parse_ip(const u8 *ip_str, u8 *out_ip);

int ip_string(const u8 *ip_addr, u8 *out_str, u32 out_str_len);

int print_ip_packet(const u8 *pkt, u32 pkt_len);

#endif
