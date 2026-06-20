#include <stdlib.h> /* malloc, free */
#include <stdio.h>  /* printf, sprintf */
#include <string.h> /* memset, memcpy */
#include "net.h"    /* write_be16, write_be32, read_be32, checksum */
#include "ip.h"     /* ip_hdr, IP_*, u8, u16, u32 */

int ip_build_packet(u8 protocol, u32 src, u32 dst, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  static u16 g_ip_id = 1;
  u8 *buf = NULL;
  ip_hdr *hdr = NULL;

  if (!pkt || !pkt_len)
  {
    return ERR;
  }

  if (pld_len > IP_MAX_PLD_SIZE)
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
  write_be16(hdr->tot_len, (u16)*pkt_len);
  write_be16(hdr->id, g_ip_id++);
  hdr->flags = 0;
  hdr->frag_off = 0;
  hdr->ttl = IP_DEFAULT_TTL;
  hdr->protocol = protocol;
  /* checksum = 0 before computing */
  write_be32(hdr->src, src);
  write_be32(hdr->dst, dst);

  write_be16(hdr->checksum, checksum(buf, IP_HDR_SIZE));

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
  u32 val;
  const u8 *p = ip;
  u8 bytes[4];

  if (!ip || !out)
  {
    return ERR;
  }

  *out = 0;

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

  *out = read_be32(bytes);

  return OK;
}

int ip_string(u32 ip, u8 *out, u32 out_len)
{
  int i = 0;
  int len = 0;
  u8 byte;

  if (!out || out_len < (u32)IP_STR_MAX_LEN)
  {
    return ERR;
  }

  for (i = 0; i < 4; i++)
  {
    byte = (ip >> (24 - i * 8)) & 0xFF;
    len += sprintf((char *)(out + len), "%d", byte);
    if (i < 3)
    {
      out[len++] = '.';
    }
  }

  out[len] = '\0';

  return OK;
}

int print_ip_packet(const u8 *pkt, u32 pkt_len)
{
  const ip_hdr *hdr = NULL;
  u32 i;
  u8 src_str[16] = {0};
  u8 dst_str[16] = {0};

  if (!pkt || pkt_len < IP_HDR_SIZE)
  {
    return ERR;
  }

  hdr = (const ip_hdr *)pkt;

  printf("-- IP HEADER --\n");
  printf("ver: %d\n", hdr->ver);
  printf("ihl: %d\n", hdr->ihl);
  printf("dscp: %d\n", hdr->dscp);
  printf("ecn: %d\n", hdr->ecn);
  printf("tot_len: %d\n", read_be16(hdr->tot_len));
  printf("id: %d\n", read_be16(hdr->id));
  printf("flags: %d\n", hdr->flags);
  printf("frag_off: %d\n", hdr->frag_off);
  printf("ttl: %d\n", hdr->ttl);
  printf("protocol: %d\n", hdr->protocol);
  ip_string(read_be32(hdr->src), src_str, sizeof(src_str));
  printf("src: %s\n", src_str);
  ip_string(read_be32(hdr->dst), dst_str, sizeof(dst_str));
  printf("dst: %s\n", dst_str);
  printf("checksum: 0x%04x\n", read_be16(hdr->checksum));
  printf("-- IP HEADER --\n");

  printf("-- IP PAYLOAD --\n");
  for (i = IP_HDR_SIZE; i < pkt_len; i++)
  {
    printf("%02x ", pkt[i]);
  }
  printf("\n");
  printf("-- IP PAYLOAD --\n");

  /* checksum over received header: 0 = valid */
  printf("checksum validation: %d\n", checksum(hdr, IP_HDR_SIZE));

  return OK;
}