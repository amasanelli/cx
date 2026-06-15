#include <stdio.h>  /* printf, perror, fprintf */
#include <stdlib.h> /* free */
#include <unistd.h> /* close */
#include "icmp.h"   /* build_ping_packet */
#include "ip.h"     /* build_ip_icmp_packet, parse_ip */
#include "socket.h" /* open_raw_ip_socket, send_packet */

int main(int argc, char **argv)
{
  u8 pld[] = "hello";
  u8 *icmp_pkt = NULL;
  u32 icmp_len;
  u8 *ip_pkt = NULL;
  u32 ip_len;
  u32 src = 0; /* kernel fills src when 0.0.0.0 */
  u32 dst;
  u8 *addr = NULL;
  u32 addr_len;
  int skt;
  u32 i;

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
    return 1;
  }

  for (i = 0; i < icmp_len; i++)
  {
    printf("%02x ", icmp_pkt[i]);
  }
  printf("\n");

  if (build_ip_icmp_packet(src, dst, icmp_pkt, icmp_len, &ip_pkt, &ip_len) != OK)
  {
    free(icmp_pkt);
    return 1;
  }

  free(icmp_pkt);

  for (i = 0; i < ip_len; i++)
  {
    printf("%02x ", ip_pkt[i]);
  }
  printf("\n");

  if (open_raw_ip_socket(&skt) != OK)
  {
    perror("socket");
    free(ip_pkt);
    return 1;
  }

  if (build_socket_address(dst, 0, &addr, &addr_len) != OK)
  {
    free(ip_pkt);
    close(skt);
    return 1;
  }

  if (send_packet(skt, addr, addr_len, ip_pkt, ip_len) != OK)
  {
    perror("sendto");
    free(addr);
    free(ip_pkt);
    close(skt);
    return 1;
  }

  free(addr);
  free(ip_pkt);
  close(skt);

  return 0;
}
