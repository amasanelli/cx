#include <netinet/in.h> /* IPPROTO_ICMP, IPPROTO_RAW, AF_INET */
#include <netinet/ip.h> /* IP_HDRINCL */
#include <stdlib.h>     /* malloc */
#include <string.h>     /* memset */
#include <sys/socket.h> /* socket, sendto, setsockopt, SOCK_RAW, sockaddr, ssize_t */
#include <unistd.h>     /* close */
#include "net.h"        /* write_be32 */
#include "socket.h"     /* u8, u32 */

int build_socket_address(u32 ip, u16 port, u8 **addr, u32 *addr_len)
{
  u8 *buf = NULL;
  skt_addr *a = NULL;

  if (!addr || !addr_len)
  {
    return ERR;
  }

  *addr = NULL;
  *addr_len = sizeof(skt_addr);

  buf = (u8 *)malloc(*addr_len);
  if (!buf)
  {
    return ERR;
  }

  a = (skt_addr *)buf;
  memset(a, 0, sizeof(skt_addr));
  a->family = AF_INET;
  a->port = port;
  write_be32(a->ip, ip);

  *addr = buf;

  return OK;
}

int open_raw_icmp_socket(int *skt)
{
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

  return OK;
}

int open_raw_ip_socket(int *skt)
{
  int one = 1;

  if (!skt)
  {
    return ERR;
  }

  *skt = -1;
  *skt = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

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

int send_packet(int skt, const u8 *addr, u32 addr_len, const u8 *pkt, u32 pkt_len)
{
  ssize_t sent;

  if (!addr || !pkt)
  {
    return ERR;
  }

  sent = sendto(skt, pkt, pkt_len, 0, (struct sockaddr *)addr, addr_len);

  if (sent < 0 || (u32)sent != pkt_len)
  {
    return ERR;
  }

  return OK;
}
