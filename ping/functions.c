#include "ping.h"
#include <time.h> /* clock_gettime, struct timespec, CLOCK_MONOTONIC */

int ping(int skt, const skt_addr *addr, u32 src, u32 dst, const u8 *src_mac, const u8 *dst_mac, u16 id, u16 seq, const u8 *pld, u32 pld_len, long *rtt_ms)
{
  u8 *icmp_pkt = NULL;
  u32 icmp_len = 0;
  u8 *ip_pkt = NULL;
  u32 ip_len = 0;
  u8 *eth_pkt = NULL;
  u32 eth_len = 0;

  u8 buf[ETH_HDR_SIZE + ETH_MAX_PLD_SIZE] = {0};
  u32 rec = 0;
  const eth_hdr *ethhdr = NULL;
  const ip_hdr *iphdr = NULL;
  const icmp_hdr *icmphdr = NULL;
  u32 ip_hdr_len = 0;

  struct timespec t_send = {0};
  struct timespec t_recv = {0};

  if (!addr || !src_mac || !dst_mac || !rtt_ms)
  {
    return ERR;
  }

  if (build_ping_packet(id, seq, pld, pld_len, &icmp_pkt, &icmp_len) != OK)
  {
    return ERR;
  }

  if (build_ip_icmp_packet(src, dst, icmp_pkt, icmp_len, &ip_pkt, &ip_len) != OK)
  {
    free(icmp_pkt);
    return ERR;
  }

  if (build_eth_ip_packet(dst_mac, src_mac, ip_pkt, ip_len, &eth_pkt, &eth_len) != OK)
  {
    free(icmp_pkt);
    free(ip_pkt);
    return ERR;
  }

  printf("----- SENT (seq=%d) -----\n", seq);
  print_eth_packet(eth_pkt, eth_len);
  print_ip_packet(ip_pkt, ip_len);
  print_icmp_packet(icmp_pkt, icmp_len);
  printf("\n");

  free(icmp_pkt);
  icmp_pkt = NULL;
  free(ip_pkt);
  ip_pkt = NULL;

  clock_gettime(CLOCK_MONOTONIC, &t_send);

  if (send_packet(skt, addr, eth_pkt, eth_len) != OK)
  {
    free(eth_pkt);
    return ERR;
  }

  free(eth_pkt);
  eth_pkt = NULL;

  /* filter frames until matching ICMP echo reply or timeout */
  for (;;)
  {
    memset(buf, 0, sizeof(buf));

    if (receive_packet(skt, buf, sizeof(buf), &rec) != OK)
    {
      return ERR; /* timeout */
    }

    if (rec < ETH_HDR_SIZE + IP_HDR_SIZE + ICMP_HDR_SIZE)
    {
      continue;
    }

    ethhdr = (const eth_hdr *)buf;
    if (read_be16(ethhdr->ethertype) != ETHER_TYPE_IP)
    {
      continue;
    }

    iphdr = (const ip_hdr *)(buf + ETH_HDR_SIZE);
    if (iphdr->ihl < 5)
    {
      continue;
    }

    ip_hdr_len = iphdr->ihl * 4;

    if (rec < ETH_HDR_SIZE + ip_hdr_len + ICMP_HDR_SIZE)
    {
      continue;
    }

    if (iphdr->protocol != IP_PROTO_ICMP)
    {
      continue;
    }

    if (read_be32(iphdr->src) != dst)
    {
      continue;
    }

    icmphdr = (const icmp_hdr *)(buf + ETH_HDR_SIZE + ip_hdr_len);
    if (icmphdr->type != ICMP_ECHO_REPLY)
    {
      continue;
    }

    if (read_be16(icmphdr->id) != id)
    {
      continue;
    }

    clock_gettime(CLOCK_MONOTONIC, &t_recv);
    break;
  }

  *rtt_ms = (t_recv.tv_sec - t_send.tv_sec) * 1000 + (t_recv.tv_nsec - t_send.tv_nsec) / 1000000;

  printf("----- RECEIVED (seq=%d) -----\n", seq);
  print_eth_packet(buf, rec);
  print_ip_packet(buf + ETH_HDR_SIZE, rec - ETH_HDR_SIZE);
  print_icmp_packet(buf + ETH_HDR_SIZE + ip_hdr_len, rec - ETH_HDR_SIZE - ip_hdr_len);
  printf("rtt: %ld ms\n\n", *rtt_ms);

  return OK;
}
