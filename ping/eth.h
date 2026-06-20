#ifndef ETH_H
#define ETH_H

#include <stdlib.h> /* malloc */
#include <stdio.h>  /* printf */
#include <string.h> /* memset, memcpy */
#include "types.h"  /* u8, u16, u32 */
#include "net.h"    /* write_be16, read_be16 */

#define ETH_HDR_SIZE 14
#define ETH_ADDR_LEN 6
#define ETH_MAX_PLD_SIZE 1500
#define ETHER_TYPE_IP 0x0800

typedef struct __attribute__((packed))
{
  u8 dst[ETH_ADDR_LEN]; /* destination MAC address */
  u8 src[ETH_ADDR_LEN]; /* source MAC address */
  u8 ethertype[2];      /* payload protocol type, network byte order */
} eth_hdr;

int eth_build_packet(const u8 *dst, const u8 *src, u16 ethertype, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len);

int build_eth_ip_packet(const u8 *dst, const u8 *src, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len);

int print_eth_packet(const u8 *pkt, u32 pkt_len);

#endif
