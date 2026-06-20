#include <stdio.h>  /* printf, perror, fprintf */
#include <stdlib.h> /* free */
#include <unistd.h> /* close */
#include <string.h> /* memset */
#include "icmp.h"   /* build_ping_packet */
#include "ip.h"     /* build_ip_icmp_packet, parse_ip */
#include "socket.h" /* open_raw_icmp_socket, send_packet */
#include "net.h"

int main(int argc, char **argv)
{
  u8 pld[] = "hello";
  u8 *icmp_pkt = NULL;
  u32 icmp_len = 0;
  u8 *ip_pkt = NULL;
  u32 ip_len = 0;
  u32 src = 0; /* kernel fills src when 0.0.0.0 */
  u32 dst = 0;
  skt_addr addr = {0};
  int skt = -1;
  u8 buf[1500] = {0};
  u32 rec = 0;
  ip_hdr *hdr = NULL;
  u32 ip_hdr_len = 0;

  if (argc != 2)
  {
    printf("usage:\n./ping IP\n");
    return 1;
  }

  if (parse_ip((const u8 *)argv[1], &dst) != OK)
  {
    fprintf(stderr, "invalid IP: %s\n", argv[1]);
    return 1;
  }

  if (build_ping_packet(1234, 1, pld, sizeof(pld) - 1, &icmp_pkt, &icmp_len) != OK)
  {
    perror("build_ping_packet");
    return 1;
  }

  if (build_ip_icmp_packet(src, dst, icmp_pkt, icmp_len, &ip_pkt, &ip_len) != OK)
  {
    perror("build_ip_icmp_packet");
    free(icmp_pkt);
    return 1;
  }

  printf("----- SENT -----\n");
  if (print_ip_packet(ip_pkt, ip_len) != OK)
  {
    fprintf(stderr, "error printing IP packet\n");
    return 1;
  }
  if (print_icmp_packet(icmp_pkt, icmp_len) != OK)
  {
    fprintf(stderr, "rerror printing ICMP packet\n");
    return 1;
  }
  printf("\n");

  free(icmp_pkt);

  if (open_raw_icmp_socket(&skt) != OK)
  {
    perror("open_raw_icmp_socket");
    free(ip_pkt);
    return 1;
  }

  if (build_socket_address(dst, 0, &addr) != OK)
  {
    perror("build_socket_address");
    free(ip_pkt);
    close(skt);
    return 1;
  }

  if (send_packet(skt, &addr, ip_pkt, ip_len) != OK)
  {
    perror("send_packet");
    free(ip_pkt);
    close(skt);
    return 1;
  }

  free(ip_pkt);

  memset(buf, 0, sizeof(buf));

  if (receive_packet(skt, buf, sizeof(buf), &rec) != OK)
  {
    perror("receive_packet");
    close(skt);
    return 1;
  }

  close(skt);

  if (rec < IP_HDR_SIZE)
  {
    fprintf(stderr, "reply too short for IP header\n");
    return 1;
  }

  hdr = (ip_hdr *)buf;

  if (hdr->ihl < 5)
  {
    fprintf(stderr, "invalid IP header length: %u\n", hdr->ihl);
    return 1;
  }

  ip_hdr_len = hdr->ihl * 4;

  if (rec < ip_hdr_len + ICMP_HDR_SIZE)
  {
    fprintf(stderr, "reply too short for ICMP header\n");
    return 1;
  }

  printf("----- RECEIVED -----\n");
  if (print_ip_packet(buf, rec) != OK)
  {
    fprintf(stderr, "error printing IP packet\n");
    return 1;
  }
  if (print_icmp_packet(buf + ip_hdr_len, rec - ip_hdr_len) != OK)
  {
    fprintf(stderr, "rerror printing ICMP packet\n");
    return 1;
  }

  return 0;
}
