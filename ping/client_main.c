#include "functions.h"

int main(int argc, char **argv)
{
  u8 pld[] = "hello";
  u8 dst_ip[IP_ADDR_LEN] = {0};
  u8 arp_ip_dst[IP_ADDR_LEN] = {0};
  u8 dst_mac[ETH_ADDR_LEN] = {0};
  iface_info iface = {0};
  skt_addr addr = {0};
  int skt = -1;
  u32 i = 0;
  bool on_link = TRUE;

  u16 seq = 0;
  u32 sent = 0;
  u32 received = 0;
  u64 rtt_ms = 0;
  u64 rtt_min = 0;
  u64 rtt_max = 0;
  u64 rtt_sum = 0;

  if (argc < 2)
  {
    printf("usage:\n./ping <IP> [iface]\n");
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

  if (get_iface_info(dst_ip, &iface) != OK)
  {
    fprintf(stderr, "no route to host: %s\n", argv[1]);
    close(skt);
    return 1;
  }
  printf("using interface: %s\n\n", iface.name);

  if (memcmp(dst_ip, iface.ip, IP_ADDR_LEN) == 0)
  {
    fprintf(stderr, "cannot ping own address via raw ethernet socket\n");
    close(skt);
    return 1;
  }

  if (build_socket_address(iface.index, &addr) != OK)
  {
    perror("build_socket_address");
    close(skt);
    return 1;
  }

  on_link = TRUE;
  for (i = 0; i < (u32)IP_ADDR_LEN; i++)
  {
    if ((dst_ip[i] & iface.netmask[i]) != (iface.ip[i] & iface.netmask[i]))
    {
      on_link = FALSE;
      break;
    }
  }
  if (on_link)
  {
    memcpy(arp_ip_dst, dst_ip, IP_ADDR_LEN);
  }
  else
  {
    memcpy(arp_ip_dst, iface.gateway, IP_ADDR_LEN);
  }

  if (set_socket_timeouts(skt, ARP_TIMEOUT_SEC) != OK)
  {
    perror("set_socket_timeouts");
    close(skt);
    return 1;
  }

  if (arp_resolve(skt, &addr, iface.ip, iface.mac, arp_ip_dst, dst_mac) != OK)
  {
    fprintf(stderr, "ARP resolution timed out\n");
    close(skt);
    return 1;
  }

  if (set_socket_timeouts(skt, PING_TIMEOUT_SEC) != OK)
  {
    perror("set_socket_timeouts");
    close(skt);
    return 1;
  }

  for (seq = 1; seq <= PING_COUNT; seq++)
  {
    sent++;

    if (ping(skt, &addr, iface.ip, dst_ip, iface.mac, dst_mac, (u16)getpid(), seq, pld, sizeof(pld) - 1, &rtt_ms) != OK)
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
