#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <time.h>   /* clock_gettime, struct timespec, CLOCK_MONOTONIC */
#include "config.h" /* PING_COUNT, PING_TIMEOUT_SEC, ARP_TIMEOUT_SEC */
#include "icmp.h"   /* build_ping_packet, print_icmp_packet, ICMP_ECHO_REPLY */
#include "ip.h"     /* build_ip_icmp_packet, print_ip_packet, IP_PROTO_ICMP */
#include "eth.h"    /* build_eth_ip_packet, print_eth_packet, ETH_MAX_PLD_SIZE, ETHER_TYPE_IP */
#include "socket.h" /* send_packet, receive_packet, skt_addr */
#include "arp.h"    /* arp_build_packet, arp_hdr, ARP_HW_ETHER, ARP_OPCODE_REPLY */

/* send ICMP echo request and wait for reply; writes round-trip time in ms into out_rtt_ms */
int ping(int skt, const skt_addr *addr, const u8 *src_ip, const u8 *dst_ip, const u8 *src_mac, const u8 *dst_mac, u16 id, u16 seq, const u8 *pld, u32 pld_len, long *out_rtt_ms);

/* send ARP request and wait for reply; writes resolved MAC into out_dst_mac */
int arp_resolve(int skt, const skt_addr *addr, const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, u8 *out_dst_mac);

#endif
