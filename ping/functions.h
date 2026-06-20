#ifndef PING_H
#define PING_H

#include "icmp.h"   /* build_ping_packet, print_icmp_packet, ICMP_ECHO_REPLY, ICMP_HDR_SIZE */
#include "ip.h"     /* build_ip_icmp_packet, print_ip_packet, IP_HDR_SIZE, IP_PROTO_ICMP */
#include "eth.h"    /* build_eth_ip_packet, print_eth_packet, ETH_HDR_SIZE, ETH_MAX_PLD_SIZE, ETHER_TYPE_IP */
#include "socket.h" /* send_packet, receive_packet, skt_addr */

int ping(int skt, const skt_addr *addr, u32 src, u32 dst, const u8 *src_mac, const u8 *dst_mac, u16 id, u16 seq, const u8 *pld, u32 pld_len, long *rtt_ms);

#endif
