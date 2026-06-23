#ifndef ICMP_H
#define ICMP_H

#include <stdlib.h> /* malloc */
#include <stdio.h>  /* printf */
#include <string.h> /* memset, memcpy */
#include "types.h"  /* u8, u16, u32 */
#include "net.h"    /* write_be16, read_be16, checksum */

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0
/* max IP payload (65535 - 20 byte IP header) minus ICMP header */
#define ICMP_MAX_PLD_SIZE (65535 - 20 - (u32)sizeof(icmp_hdr))

typedef struct __attribute__((packed))
{
  u8 type;        /* message type (ICMP_ECHO_REQUEST, ICMP_ECHO_REPLY, ...) */
  u8 code;        /* subtype within the message type */
  u8 checksum[2]; /* header + data checksum, network byte order */
  u8 id[2];       /* identifies the echo session, network byte order */
  u8 seq[2];      /* sequence number to match replies, network byte order */
} icmp_hdr;

int icmp_build_packet(u8 type, u8 code, u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len);

int build_ping_packet(u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len);

int build_pong_packet(u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len);

int print_icmp_packet(const u8 *pkt, u32 pkt_len);

#endif
