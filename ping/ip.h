#ifndef IP_H
#define IP_H

#include <endian.h>
#include "types.h" /* u8, u16, u32 */

#define IP_DEFAULT_TTL 64
#define IP_HDR_SIZE 20
#define IP_MAX_PLD_SIZE (65535 - IP_HDR_SIZE)
#define IP_PROTO_ICMP 1
#define IP_STR_MAX_LEN 16 /* "255.255.255.255\0" */

typedef struct __attribute__((packed))
{
#if __BYTE_ORDER == __BIG_ENDIAN
  u32 ver : 4;
  u32 ihl : 4;
#else
  u32 ihl : 4;
  u32 ver : 4;
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
  u32 dscp : 6;
  u32 ecn : 2;
#else
  u32 ecn : 2;
  u32 dscp : 6;
#endif
  u8 tot_len[2];
  u8 id[2];
#if __BYTE_ORDER == __BIG_ENDIAN
  u32 flags : 3;
  u32 frag_off : 13;
#else
  u32 frag_off : 13;
  u32 flags : 3;
#endif
  u8 ttl;
  u8 protocol;
  u8 checksum[2];
  u8 src[4];
  u8 dst[4];
} ip_hdr;

int ip_build_packet(u8 protocol, u32 src, u32 dst, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len);

int build_ip_icmp_packet(u32 src, u32 dst, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len);

int parse_ip(const u8 *ip, u32 *out);

int ip_string(u32 ip, u8 *out, u32 out_len);

int print_ip_packet(const u8 *pkt, u32 pkt_len);

#endif
