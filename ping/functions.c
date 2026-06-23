#include "functions.h"

int ping(int skt, const skt_addr *addr, const u8 *src_ip, const u8 *dst_ip, const u8 *src_mac, const u8 *dst_mac, u16 id, u16 seq, const u8 *pld, u32 pld_len, u64 *out_rtt_ms)
{
  u8 *icmp_pkt = NULL;
  u32 icmp_len = 0;
  u8 *ip_pkt = NULL;
  u32 ip_len = 0;
  u8 *eth_pkt = NULL;
  u32 eth_len = 0;

  u8 buf[sizeof(eth_hdr) + ETH_MAX_PLD_SIZE] = {0};
  u32 n_recv = 0;
  const eth_hdr *ethhdr = NULL;
  const ip_hdr *iphdr = NULL;
  const icmp_hdr *icmphdr = NULL;
  u32 ip_hdr_len = 0;

  struct timespec t_send = {0};
  struct timespec t_recv = {0};

  if (!addr || !src_ip || !dst_ip || !src_mac || !dst_mac || !out_rtt_ms)
  {
    return ERR;
  }

  if (build_ping_packet(id, seq, pld, pld_len, &icmp_pkt, &icmp_len) != OK)
  {
    return ERR;
  }

  if (build_ip_icmp_packet(src_ip, dst_ip, icmp_pkt, icmp_len, &ip_pkt, &ip_len) != OK)
  {
    free(icmp_pkt);
    icmp_pkt = NULL;
    return ERR;
  }

  if (build_eth_ip_packet(dst_mac, src_mac, ip_pkt, ip_len, &eth_pkt, &eth_len) != OK)
  {
    free(icmp_pkt);
    icmp_pkt = NULL;
    free(ip_pkt);
    ip_pkt = NULL;
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
    eth_pkt = NULL;
    return ERR;
  }

  free(eth_pkt);
  eth_pkt = NULL;

  /* filter frames until matching ICMP echo reply or timeout */
  for (;;)
  {
    memset(buf, 0, sizeof(buf));

    if (receive_packet(skt, buf, sizeof(buf), &n_recv) != OK)
    {
      return ERR; /* timeout */
    }

    if (n_recv < (u32)(sizeof(eth_hdr) + sizeof(ip_hdr) + sizeof(icmp_hdr)))
    {
      continue;
    }

    ethhdr = (const eth_hdr *)buf;
    if (read_be16(ethhdr->ethertype) != ETHER_TYPE_IP)
    {
      continue;
    }

    iphdr = (const ip_hdr *)(buf + sizeof(eth_hdr));
    if (iphdr->ihl < 5)
    {
      continue;
    }

    ip_hdr_len = iphdr->ihl * 4;

    if (n_recv < (u32)sizeof(eth_hdr) + ip_hdr_len + (u32)sizeof(icmp_hdr))
    {
      continue;
    }

    if (iphdr->protocol != IP_PROTO_ICMP)
    {
      continue;
    }

    if (memcmp(iphdr->src, dst_ip, IP_ADDR_LEN) != 0)
    {
      continue;
    }

    icmphdr = (const icmp_hdr *)(buf + sizeof(eth_hdr) + ip_hdr_len);
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

  *out_rtt_ms = (u64)((t_recv.tv_sec - t_send.tv_sec) * 1000 + (t_recv.tv_nsec - t_send.tv_nsec) / 1000000);

  printf("----- RECEIVED (seq=%d) -----\n", seq);
  print_eth_packet(buf, n_recv);
  print_ip_packet(buf + sizeof(eth_hdr), n_recv - (u32)sizeof(eth_hdr));
  print_icmp_packet(buf + sizeof(eth_hdr) + ip_hdr_len, n_recv - (u32)sizeof(eth_hdr) - ip_hdr_len);
  printf("rtt: %lu ms\n\n", *out_rtt_ms);

  return OK;
}

int arp_resolve(int skt, const skt_addr *addr, const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, u8 *out_dst_mac)
{
  u8 *arp_pkt = NULL;
  u32 arp_len = 0;
  u8 *eth_pkt = NULL;
  u32 eth_len = 0;
  u8 broadcast[ETH_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  u8 buf[sizeof(eth_hdr) + ETH_MAX_PLD_SIZE] = {0};
  u32 n_recv = 0;
  const eth_hdr *ethhdr = NULL;
  const arp_hdr *arphdr = NULL;

  if (!addr || !src_ip || !src_mac || !dst_ip || !out_dst_mac)
  {
    return ERR;
  }

  memset(out_dst_mac, 0, ETH_ADDR_LEN);

  if (arp_build_request_packet(src_ip, src_mac, dst_ip, &arp_pkt, &arp_len) != OK)
  {
    return ERR;
  }

  if (build_eth_arp_packet(broadcast, src_mac, arp_pkt, arp_len, &eth_pkt, &eth_len) != OK)
  {
    free(arp_pkt);
    arp_pkt = NULL;
    return ERR;
  }

  printf("----- ARP SENT -----\n");
  print_eth_packet(eth_pkt, eth_len);
  print_arp_packet(arp_pkt, arp_len);
  printf("\n");

  free(arp_pkt);
  arp_pkt = NULL;

  if (send_packet(skt, addr, eth_pkt, eth_len) != OK)
  {
    free(eth_pkt);
    eth_pkt = NULL;
    return ERR;
  }

  free(eth_pkt);
  eth_pkt = NULL;

  /* filter frames until matching ARP reply or socket timeout */
  for (;;)
  {
    memset(buf, 0, sizeof(buf));

    if (receive_packet(skt, buf, sizeof(buf), &n_recv) != OK)
    {
      return ERR; /* timeout */
    }

    if (n_recv < (u32)(sizeof(eth_hdr) + sizeof(arp_hdr)))
    {
      continue;
    }

    ethhdr = (const eth_hdr *)buf;
    if (read_be16(ethhdr->ethertype) != ETHER_TYPE_ARP)
    {
      continue;
    }

    arphdr = (const arp_hdr *)(buf + sizeof(eth_hdr));

    if (read_be16(arphdr->hw_type) != ARP_HW_ETHER)
    {
      continue;
    }

    if (read_be16(arphdr->opcode) != ARP_OPCODE_REPLY)
    {
      continue;
    }

    if (memcmp(arphdr->sender_ip, dst_ip, 4) != 0)
    {
      continue;
    }

    printf("----- ARP RECEIVED -----\n");
    print_eth_packet(buf, n_recv);
    print_arp_packet(buf + sizeof(eth_hdr), n_recv - (u32)sizeof(eth_hdr));
    printf("\n");

    memcpy(out_dst_mac, arphdr->sender_mac, ETH_ADDR_LEN);

    return OK;
  }
}

int pong(int skt)
{
  const arp_hdr *arphdr = NULL;
  const eth_hdr *ethhdr = NULL;
  const icmp_hdr *icmphdr = NULL;
  const ip_hdr *iphdr = NULL;
  const u8 *pld = NULL;
  iface_info iface = {0};
  skt_addr addr = {0};
  u16 id = 0;
  u16 seq = 0;
  u32 arp_len = 0;
  u32 eth_len = 0;
  u32 icmp_len = 0;
  u32 ip_hdr_len = 0;
  u32 ip_len = 0;
  u32 n_recv = 0;
  u32 pld_len = 0;
  u8 *arp_pkt = NULL;
  u8 *eth_pkt = NULL;
  u8 *icmp_pkt = NULL;
  u8 *ip_pkt = NULL;
  u8 buf[sizeof(eth_hdr) + ETH_MAX_PLD_SIZE] = {0};

  if (receive_packet(skt, buf, sizeof(buf), &n_recv) != OK)
  {
    return ERR;
  }

  if (n_recv < (u32)(sizeof(eth_hdr) + sizeof(arp_hdr)))
  {
    return OK;
  }

  ethhdr = (const eth_hdr *)buf;

  if (read_be16(ethhdr->ethertype) == ETHER_TYPE_ARP)
  {
    arphdr = (const arp_hdr *)(buf + sizeof(eth_hdr));

    if (read_be16(arphdr->hw_type) != ARP_HW_ETHER)
    {
      return OK;
    }
    if (read_be16(arphdr->proto_type) != ARP_PROTO_IP)
    {
      return OK;
    }
    if (read_be16(arphdr->opcode) != ARP_OPCODE_REQUEST)
    {
      return OK;
    }

    if (get_iface_info(arphdr->target_ip, &iface) != OK)
    {
      return OK;
    }
    if (memcmp(arphdr->target_ip, iface.ip, IP_ADDR_LEN) != 0)
    {
      return OK;
    }

    if (build_socket_address(iface.index, &addr) != OK)
    {
      return ERR;
    }

    printf("----- ARP REQUEST -----\n");
    print_eth_packet(buf, n_recv);
    print_arp_packet(buf + sizeof(eth_hdr), n_recv - (u32)sizeof(eth_hdr));
    printf("\n");

    if (arp_build_reply_packet(iface.ip, iface.mac, arphdr->sender_ip, arphdr->sender_mac, &arp_pkt, &arp_len) != OK)
    {
      return ERR;
    }

    if (build_eth_arp_packet(arphdr->sender_mac, iface.mac, arp_pkt, arp_len, &eth_pkt, &eth_len) != OK)
    {
      free(arp_pkt);
      arp_pkt = NULL;
      return ERR;
    }

    free(arp_pkt);
    arp_pkt = NULL;

    printf("----- ARP REPLY -----\n");
    print_eth_packet(eth_pkt, eth_len);
    print_arp_packet(eth_pkt + sizeof(eth_hdr), eth_len - (u32)sizeof(eth_hdr));
    printf("\n");

    if (send_packet(skt, &addr, eth_pkt, eth_len) != OK)
    {
      free(eth_pkt);
      eth_pkt = NULL;
      return ERR;
    }

    free(eth_pkt);
    eth_pkt = NULL;
    return OK;
  }

  if (read_be16(ethhdr->ethertype) != ETHER_TYPE_IP)
  {
    return OK;
  }
  if (n_recv < (u32)(sizeof(eth_hdr) + sizeof(ip_hdr) + sizeof(icmp_hdr)))
  {
    return OK;
  }

  iphdr = (const ip_hdr *)(buf + sizeof(eth_hdr));
  if (iphdr->ihl < 5)
  {
    return OK;
  }
  ip_hdr_len = iphdr->ihl * 4U;
  if (n_recv < (u32)sizeof(eth_hdr) + ip_hdr_len + (u32)sizeof(icmp_hdr))
  {
    return OK;
  }
  if (iphdr->protocol != IP_PROTO_ICMP)
  {
    return OK;
  }

  icmphdr = (const icmp_hdr *)(buf + sizeof(eth_hdr) + ip_hdr_len);
  if (icmphdr->type != ICMP_ECHO_REQUEST)
  {
    return OK;
  }

  if (get_iface_info(iphdr->dst, &iface) != OK)
  {
    return OK;
  }
  if (memcmp(iphdr->dst, iface.ip, IP_ADDR_LEN) != 0)
  {
    return OK;
  }
  if (build_socket_address(iface.index, &addr) != OK)
  {
    return ERR;
  }

  id = read_be16(icmphdr->id);
  seq = read_be16(icmphdr->seq);
  icmp_len = n_recv - (u32)sizeof(eth_hdr) - ip_hdr_len;
  pld = buf + sizeof(eth_hdr) + ip_hdr_len + sizeof(icmp_hdr);
  pld_len = icmp_len - (u32)sizeof(icmp_hdr);

  printf("----- REQUEST (seq=%d) -----\n", seq);
  print_eth_packet(buf, n_recv);
  print_ip_packet(buf + (u32)sizeof(eth_hdr), n_recv - (u32)sizeof(eth_hdr));
  print_icmp_packet(buf + (u32)sizeof(eth_hdr) + ip_hdr_len, icmp_len);
  printf("\n");

  if (build_pong_packet(id, seq, pld, pld_len, &icmp_pkt, &icmp_len) != OK)
  {
    return ERR;
  }

  if (build_ip_icmp_packet(iface.ip, iphdr->src, icmp_pkt, icmp_len, &ip_pkt, &ip_len) != OK)
  {
    free(icmp_pkt);
    icmp_pkt = NULL;
    return ERR;
  }

  free(icmp_pkt);
  icmp_pkt = NULL;

  if (build_eth_ip_packet(ethhdr->src, iface.mac, ip_pkt, ip_len, &eth_pkt, &eth_len) != OK)
  {
    free(ip_pkt);
    ip_pkt = NULL;
    return ERR;
  }

  free(ip_pkt);
  ip_pkt = NULL;

  ip_hdr_len = ((const ip_hdr *)(eth_pkt + sizeof(eth_hdr)))->ihl * 4U;

  printf("----- REPLY (seq=%d) -----\n", seq);
  print_eth_packet(eth_pkt, eth_len);
  print_ip_packet(eth_pkt + (u32)sizeof(eth_hdr), eth_len - (u32)sizeof(eth_hdr));
  print_icmp_packet(eth_pkt + (u32)sizeof(eth_hdr) + ip_hdr_len, eth_len - (u32)sizeof(eth_hdr) - ip_hdr_len);
  printf("\n");

  if (send_packet(skt, &addr, eth_pkt, eth_len) != OK)
  {
    free(eth_pkt);
    eth_pkt = NULL;
    return ERR;
  }

  free(eth_pkt);
  eth_pkt = NULL;
  return OK;
}
