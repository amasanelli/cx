#ifndef IP_H
#define IP_H

#include <endian.h>
#include "types.h" /* u8, u16, u32 */

#define IP_DEFAULT_TTL 64
#define IP_HDR_SIZE 20
#define IP_PROTO_ICMP 1

typedef struct __attribute__((packed))
{
#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned int ver : 4;
  unsigned int ihl : 4;
#else
  unsigned int ihl : 4;
  unsigned int ver : 4;
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned int dscp : 6;
  unsigned int ecn : 2;
#else
  unsigned int ecn : 2;
  unsigned int dscp : 6;
#endif
  u8 tot_len[2];
  u8 id[2];
#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned int flags : 3;
  unsigned int frag_off : 13;
#else
  unsigned int frag_off : 13;
  unsigned int flags : 3;
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

#endif
