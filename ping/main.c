#include "functions.h"

int main(int argc, char **argv)
{
  u8 pld[] = "hello";
  u8 src_ip[IP_ADDR_LEN] = {0};
  u8 src_ip_msk[IP_ADDR_LEN] = {0};
  u8 gw_ip[IP_ADDR_LEN] = {0};
  u8 arp_ip_dst[IP_ADDR_LEN] = {0};
  u8 src_mac[ETH_ADDR_LEN] = {0};
  u8 dst_mac[ETH_ADDR_LEN] = {0}; /* filled by arp_resolve */

  u8 dst_ip[IP_ADDR_LEN] = {0};
  u32 if_i = 0;
  skt_addr addr = {0};
  int skt = -1;

  u32 i = 0;
  bool same = TRUE;
  u16 seq = 0;
  u32 sent = 0;
  u32 received = 0;
  u64 rtt_ms = 0;
  u64 rtt_min = 0;
  u64 rtt_max = 0;
  u64 rtt_sum = 0;

  if (argc != 3)
  {
    printf("usage:\n./ping <IP> <iface>\n");
    return 1;
  }

  if (parse_ip((const u8 *)argv[1], dst_ip) != OK)
  {
    fprintf(stderr, "invalid IP: %s\n", argv[1]);
    return 1;
  }

  if (open_raw_eth_socket(&skt) != OK)
  {
    perror("open_raw_eth_socket");
    return 1;
  }

  if (get_iface_index(skt, argv[2], &if_i) != OK)
  {
    fprintf(stderr, "invalid interface: %s\n", argv[2]);
    close(skt);
    return 1;
  }

  if (get_iface_ip(skt, argv[2], src_ip) != OK)
  {
    fprintf(stderr, "failed to get IP for interface: %s\n", argv[2]);
    close(skt);
    return 1;
  }

  if (get_iface_mac(skt, argv[2], src_mac) != OK)
  {
    fprintf(stderr, "failed to get MAC for interface: %s\n", argv[2]);
    close(skt);
    return 1;
  }

  if (get_iface_netmask(skt, argv[2], src_ip_msk) != OK)
  {
    fprintf(stderr, "failed to get netmask for interface: %s\n", argv[2]);
    close(skt);
    return 1;
  }

  if (build_socket_address(if_i, &addr) != OK)
  {
    perror("build_socket_address");
    close(skt);
    return 1;
  }

  /* if dst_ip is on the same subnet, ARP for it directly; otherwise ARP for the gateway */
  for (i = 0; i < IP_ADDR_LEN; i++)
  {
    if ((dst_ip[i] & src_ip_msk[i]) != (src_ip[i] & src_ip_msk[i]))
    {
      same = FALSE;
      break;
    }
  }
  if (same)
  {
    memcpy(arp_ip_dst, dst_ip, IP_ADDR_LEN);
  }
  else
  {
    if (get_iface_gateway(argv[2], gw_ip) != OK)
    {
      fprintf(stderr, "no default gateway found for interface: %s\n", argv[2]);
      close(skt);
      return 1;
    }
    memcpy(arp_ip_dst, gw_ip, IP_ADDR_LEN);
  }

  if (set_recv_timeout(skt, ARP_TIMEOUT_SEC) != OK)
  {
    perror("set_recv_timeout");
    close(skt);
    return 1;
  }

  if (arp_resolve(skt, &addr, src_ip, src_mac, arp_ip_dst, dst_mac) != OK)
  {
    fprintf(stderr, "ARP resolution timed out\n");
    close(skt);
    return 1;
  }

  if (set_recv_timeout(skt, PING_TIMEOUT_SEC) != OK)
  {
    perror("set_recv_timeout");
    close(skt);
    return 1;
  }

  for (seq = 1; seq <= PING_COUNT; seq++)
  {
    sent++;

    if (ping(skt, &addr, src_ip, dst_ip, src_mac, dst_mac, (u16)getpid(), seq, pld, sizeof(pld) - 1, &rtt_ms) != OK)
    {
      fprintf(stderr, "request timeout for seq=%u\n", seq);
      continue;
    }

    received++;
    rtt_sum += rtt_ms;
    if (received == 1 || rtt_ms < rtt_min)
    {
      rtt_min = rtt_ms;
    }
    if (rtt_ms > rtt_max)
    {
      rtt_max = rtt_ms;
    }
  }

  close(skt);

  printf("----- PING STATISTICS -----\n");
  printf("%u transmitted, %u received, %u%% loss\n", sent, received, sent > 0 ? (sent - received) * 100 / sent : 0);
  if (received > 0)
  {
    printf("rtt min/avg/max = %lu/%lu/%lu ms\n", rtt_min, rtt_sum / (u64)received, rtt_max);
  }
  printf("----- PING STATISTICS -----\n");

  return 0;
}
