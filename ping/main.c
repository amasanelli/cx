#include "icmp.h"
#include "ip.h"
#include "eth.h"
#include "socket.h"

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

  u8 *icmp_pkt = NULL;
  u32 icmp_len = 0;
  u8 *ip_pkt = NULL;
  u32 ip_len = 0;
  u8 *eth_pkt = NULL;
  u32 eth_len = 0;
  skt_addr addr = {0};

  int skt = -1;

  u8 buf[ETH_HDR_SIZE + ETH_MAX_PLD_SIZE] = {0};
  u32 rec = 0;
  const ip_hdr *iphdr = NULL;
  const eth_hdr *ethhdr = NULL;
  const icmp_hdr *icmphdr = NULL;
  u32 ip_hdr_len = 0;

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

  if (get_iface_index(argv[2], &ifindex) != OK)
  {
    fprintf(stderr, "invalid interface: %s\n", argv[2]);
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

  if (build_ping_packet((u16)getpid(), 1, pld, sizeof(pld) - 1, &icmp_pkt, &icmp_len) != OK)
  {
    perror("build_ping_packet");
    close(skt);
    return 1;
  }

  if (build_ip_icmp_packet(src, dst, icmp_pkt, icmp_len, &ip_pkt, &ip_len) != OK)
  {
    perror("build_ip_icmp_packet");
    free(icmp_pkt);
    close(skt);
    return 1;
  }

  if (build_eth_ip_packet(dst_mac, src_mac, ip_pkt, ip_len, &eth_pkt, &eth_len) != OK)
  {
    perror("build_eth_ip_packet");
    free(icmp_pkt);
    free(ip_pkt);
    close(skt);
    return 1;
  }

  printf("----- SENT -----\n");
  if (print_eth_packet(eth_pkt, eth_len) != OK)
  {
    fprintf(stderr, "error printing ETH packet\n");
    return 1;
  }
  if (print_ip_packet(ip_pkt, ip_len) != OK)
  {
    fprintf(stderr, "error printing IP packet\n");
    return 1;
  }
  if (print_icmp_packet(icmp_pkt, icmp_len) != OK)
  {
    fprintf(stderr, "error printing ICMP packet\n");
    return 1;
  }
  printf("\n");

  free(icmp_pkt);
  free(ip_pkt);

  if (build_socket_address(ifindex, &addr) != OK)
  {
    perror("build_socket_address");
    free(eth_pkt);
    close(skt);
    return 1;
  }

  if (send_packet(skt, &addr, eth_pkt, eth_len) != OK)
  {
    perror("send_packet");
    free(eth_pkt);
    close(skt);
    return 1;
  }

  free(eth_pkt);

  /* loop until we receive an ICMP echo reply from the target, or timeout */
  for (;;)
  {
    memset(buf, 0, sizeof(buf));

    if (receive_packet(skt, buf, sizeof(buf), &rec) != OK)
    {
      perror("receive_packet");
      close(skt);
      return 1;
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

    if (read_be16(icmphdr->id) != (u16)getpid())
    {
      continue;
    }

    break;
  }

  close(skt);

  printf("----- RECEIVED -----\n");
  if (print_eth_packet(buf, rec) != OK)
  {
    fprintf(stderr, "error printing ETH packet\n");
    return 1;
  }
  if (print_ip_packet(buf + ETH_HDR_SIZE, rec - ETH_HDR_SIZE) != OK)
  {
    fprintf(stderr, "error printing IP packet\n");
    return 1;
  }
  if (print_icmp_packet(buf + ETH_HDR_SIZE + ip_hdr_len, rec - ETH_HDR_SIZE - ip_hdr_len) != OK)
  {
    fprintf(stderr, "error printing ICMP packet\n");
    return 1;
  }

  return 0;
}
