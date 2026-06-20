#include <netinet/in.h> /* IPPROTO_ICMP, IPPROTO_RAW, AF_INET */
#include <netinet/ip.h> /* IP_HDRINCL */
#include <stdlib.h>     /* malloc */
#include <string.h>     /* memset */
#include <sys/socket.h> /* socket, sendto, setsockopt, SOCK_RAW, sockaddr, ssize_t */
#include <unistd.h>     /* close */
#include "net.h"        /* write_be32 */
#include "socket.h"     /* u8, u32 */

int build_socket_address(u32 ip, u16 port, skt_addr *addr)
{
  if (!addr)
  {
    return ERR;
  }

  memset(addr, 0, sizeof(skt_addr));

  addr->family = AF_INET;
  write_be16(addr->port, port);
  write_be32(addr->ip, ip);

  return OK;
}

int open_raw_icmp_socket(int *skt)
{
  int one = 1;

  if (!skt)
  {
    return ERR;
  }

  *skt = -1;
  *skt = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

  if (*skt < 0)
  {
    return ERR;
  }

  if (setsockopt(*skt, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
  {
    close(*skt);
    return ERR;
  }

  return OK;
}

int send_packet(int skt, const skt_addr *addr, const u8 *pkt, u32 pkt_len)
{
  ssize_t sent = -1;

  if (!addr || !pkt)
  {
    return ERR;
  }

  sent = sendto(skt, pkt, pkt_len, 0, (struct sockaddr *)addr, sizeof(skt_addr));

  if (sent < 0 || (u32)sent != pkt_len)
  {
    return ERR;
  }

  return OK;
}

int receive_packet(int skt, u8 *buf, u32 buff_len, u32 *rec)
{
  ssize_t n = -1;

  if (!rec)
  {
    return ERR;
  }

  n = recvfrom(skt, buf, buff_len, 0, NULL, NULL);

  if (n < 0)
  {
    return ERR;
  }

  *rec = (u32)n;

  return OK;
}