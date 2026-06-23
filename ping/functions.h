#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <time.h>   /* clock_gettime, struct timespec, CLOCK_MONOTONIC */
#include "config.h" /* PING_COUNT, PING_TIMEOUT_SEC, ARP_TIMEOUT_SEC */
#include "icmp.h"   /* build_ping_packet, build_pong_packet, print_icmp_packet, icmp_hdr, ICMP_ECHO_REQUEST, ICMP_ECHO_REPLY */
#include "ip.h"     /* build_ip_icmp_packet, print_ip_packet, ip_hdr, IP_ADDR_LEN, IP_PROTO_ICMP */
#include "eth.h"    /* build_eth_ip_packet, build_eth_arp_packet, print_eth_packet, eth_hdr, ETH_ADDR_LEN, ETH_MAX_PLD_SIZE, ETHER_TYPE_IP, ETHER_TYPE_ARP */
#include "socket.h" /* send_packet, receive_packet, build_socket_address, get_iface_info, skt_addr, iface_info */
#include "arp.h"    /* arp_build_request_packet, arp_build_reply_packet, print_arp_packet, arp_hdr, ARP_HW_ETHER, ARP_PROTO_IP, ARP_OPCODE_REQUEST, ARP_OPCODE_REPLY */

/* send ICMP echo request and wait for reply; writes round-trip time in ms into out_rtt_ms */
int ping(int skt, const skt_addr *addr, const u8 *src_ip, const u8 *dst_ip, const u8 *src_mac, const u8 *dst_mac, u16 id, u16 seq, const u8 *pld, u32 pld_len, u64 *out_rtt_ms);

/* send ARP request and wait for reply; writes resolved MAC into out_dst_mac */
int arp_resolve(int skt, const skt_addr *addr, const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, u8 *out_dst_mac);

/* receive one frame; if it's an ICMP echo request for any local IP, send a reply */
int pong(int skt);

#endif
