#include "ping.h"
#include "config.h" /* PING_COUNT */

int main(int argc, char **argv)
{
  u8 pld[] = "hello";
  u32 src = 0;
  u8 src_mac[ETH_ADDR_LEN] = {0};
  /* broadcast: we skip ARP, so we don't know the target's MAC; all hosts on the
   * segment receive the frame and the target responds based on the IP address */
  u8 dst_mac[ETH_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  u32 dst = 0;
  int ifindex = 0;
  skt_addr addr = {0};
  int skt = -1;

  u16 seq = 0;
  u32 sent = 0;
  u32 received = 0;
  long rtt_ms = 0;
  long rtt_min = 0;
  long rtt_max = 0;
  long rtt_sum = 0;

  if (argc != 3)
  {
    printf("usage:\n./ping <IP> <iface>\n");
    return 1;
  }

  if (parse_ip((const u8 *)argv[1], &dst) != OK)
  {
    fprintf(stderr, "invalid IP: %s\n", argv[1]);
    return 1;
  }

  if (open_raw_eth_socket(&skt) != OK)
  {
    perror("open_raw_eth_socket");
    return 1;
  }

  if (set_recv_timeout(skt, 2) != OK)
  {
    perror("set_recv_timeout");
    close(skt);
    return 1;
  }

  if (get_iface_index(skt, argv[2], &ifindex) != OK)
  {
    fprintf(stderr, "invalid interface: %s\n", argv[2]);
    close(skt);
    return 1;
  }

  if (get_iface_ip(skt, argv[2], &src) != OK)
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

  if (build_socket_address(ifindex, &addr) != OK)
  {
    perror("build_socket_address");
    close(skt);
    return 1;
  }

  for (seq = 1; seq <= PING_COUNT; seq++)
  {
    sent++;

    if (ping(skt, &addr, src, dst, src_mac, dst_mac, (u16)getpid(), seq, pld, sizeof(pld) - 1, &rtt_ms) != OK)
    {
      fprintf(stderr, "request timeout for seq=%d\n", seq);
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
    printf("rtt min/avg/max = %ld/%ld/%ld ms\n", rtt_min, rtt_sum / (long)received, rtt_max);
  }
  printf("----- PING STATISTICS -----\n");

  return 0;
}
