#include <stdlib.h> /* malloc, free */
#include <string.h> /* memset, memcpy */
#include "net.h"    /* write_endian16, write_endian32, read_endian32, checksum */
#include "ip.h"     /* ip_hdr, IP_*, u8, u16, u32 */

int ip_build_packet(u8 protocol, u32 src, u32 dst, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  u8 *buf = NULL;
  ip_hdr *hdr = NULL;

  if (!pkt || !pkt_len)
  {
    return ERR;
  }

  if (pld_len > 0 && !pld)
  {
    return ERR;
  }

  *pkt = NULL;
  *pkt_len = IP_HDR_SIZE + pld_len;

  buf = (u8 *)malloc(*pkt_len);
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, *pkt_len);

  hdr = (ip_hdr *)buf;

  hdr->ver = 4;
  hdr->ihl = 5; /* 20 bytes, no options */
  hdr->dscp = 0;
  hdr->ecn = 0;
  write_endian16(hdr->tot_len, (u16)*pkt_len);
  write_endian16(hdr->id, 0);
  hdr->flags = 0;
  hdr->frag_off = 0;
  hdr->ttl = IP_DEFAULT_TTL;
  hdr->protocol = protocol;
  /* checksum = 0 before computing */
  write_endian32(hdr->src, src);
  write_endian32(hdr->dst, dst);

  write_endian16(hdr->checksum, checksum(buf, IP_HDR_SIZE));

  if (pld_len > 0)
  {
    memcpy(buf + IP_HDR_SIZE, pld, pld_len);
  }

  *pkt = buf;

  return OK;
}

int build_ip_icmp_packet(u32 src, u32 dst, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  return ip_build_packet(IP_PROTO_ICMP, src, dst, pld, pld_len, pkt, pkt_len);
}

int parse_ip(const u8 *ip, u32 *out)
{
  int i;
  unsigned int val;
  const u8 *p = ip;
  u8 bytes[4];

  if (!ip || !out)
  {
    return ERR;
  }

  for (i = 0; i < 4; i++)
  {
    if (*p < '0' || *p > '9')
    {
      return ERR;
    }

    val = 0;
    while (*p >= '0' && *p <= '9')
    {
      val = val * 10 + (*p - '0');
      p++;
      if (val > 255)
      {
        return ERR;
      }
    }

    if (i < 3)
    {
      if (*p != '.')
      {
        return ERR;
      }
      p++;
    }

    bytes[i] = (u8)val;
  }

  if (*p != '\0')
  {
    return ERR;
  }

  *out = read_endian32(bytes);

  return OK;
}
