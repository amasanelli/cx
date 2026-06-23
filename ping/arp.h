#ifndef ARP_H
#define ARP_H

#include <stdlib.h> /* malloc, free */
#include <stdio.h>  /* printf */
#include <string.h> /* memset, memcpy */
#include "types.h"  /* u8, u16, u32, OK, ERR */
#include "net.h"    /* write_be16, write_be32, read_be16, read_be32 */
#include "eth.h"    /* eth_build_packet, ETH_HDR_SIZE, ETH_MAX_PLD_SIZE, ETH_ADDR_LEN, ETHER_TYPE_ARP */
#include "socket.h" /* send_packet, receive_packet, skt_addr */

#define ARP_HW_ETHER 0x0001  /* hardware type: Ethernet */
#define ARP_PROTO_IP 0x0800  /* protocol type: IPv4 */
#define ARP_OPCODE_REQUEST 1 /* operation: who has <IP>? tell <sender> */
#define ARP_OPCODE_REPLY 2   /* operation: <IP> is at <MAC> */

typedef struct __attribute__((packed))
{
  u8 hw_type[2];     /* hardware type: 0x0001 = Ethernet */
  u8 proto_type[2];  /* protocol type: 0x0800 = IPv4 */
  u8 hw_addr_len;    /* hardware address length = 6 (MAC) */
  u8 proto_addr_len; /* protocol address length = 4 (IPv4) */
  u8 opcode[2];      /* operation: 1 = request, 2 = reply */
  u8 sender_mac[6];  /* sender hardware address */
  u8 sender_ip[4];   /* sender protocol address */
  u8 target_mac[6];  /* target hardware address (zeros in request) */
  u8 target_ip[4];   /* target protocol address */
} arp_hdr;

int arp_build_packet(const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, const u8 *dst_mac, u16 opcode, u8 **out_pkt, u32 *out_pkt_len);

int arp_build_request_packet(const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, u8 **out_pkt, u32 *out_pkt_len);

int arp_build_reply_packet(const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, const u8 *dst_mac, u8 **out_pkt, u32 *out_pkt_len);

int print_arp_packet(const u8 *pkt, u32 pkt_len);

#endif
