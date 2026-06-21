#include "ip.h"

int ip_build_packet(u8 protocol, const u8 *src_ip, const u8 *dst_ip, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  static u16 g_ip_id = 1;
  u8 *buf = NULL;
  ip_hdr *hdr = NULL;

  if (!out_pkt || !out_pkt_len || !src_ip || !dst_ip)
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

  *out_pkt = NULL;
  *out_pkt_len = (u32)sizeof(ip_hdr) + pld_len;

  buf = (u8 *)malloc(*out_pkt_len);
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, *out_pkt_len);

  hdr = (ip_hdr *)buf;

  hdr->ver = 4;                                /* IPv4 */
  hdr->ihl = 5;                                /* 20 bytes, no options */
  hdr->dscp = 0;                               /* default service class, no priority */
  hdr->ecn = 0;                                /* no congestion notification */
  write_be16((u16)*out_pkt_len, hdr->tot_len); /* header + payload size */
  write_be16(g_ip_id++, hdr->id);              /* unique per-packet, auto-incremented */
  hdr->flags = 0;                              /* fragmentation allowed */
  hdr->frag_off = 0;                           /* not a fragment */
  hdr->ttl = IP_DEFAULT_TTL;                   /* 64 hops */
  hdr->protocol = protocol;                    /* encapsulated protocol (passed in) */
  /* checksum = 0 before computing */
  memcpy(hdr->src, src_ip, IP_ADDR_LEN); /* sender's IP */
  memcpy(hdr->dst, dst_ip, IP_ADDR_LEN); /* target IP */

  write_be16(checksum(buf, (u32)sizeof(ip_hdr)), hdr->checksum);

  if (pld_len > 0)
  {
    memcpy(buf + sizeof(ip_hdr), pld, pld_len);
  }

  *out_pkt = buf;

  return OK;
}

int build_ip_icmp_packet(const u8 *src_ip, const u8 *dst_ip, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  return ip_build_packet(IP_PROTO_ICMP, src_ip, dst_ip, pld, pld_len, out_pkt, out_pkt_len);
}

int parse_ip(const u8 *ip_str, u8 *out_ip)
{
  u32 i = 0;
  u32 val = 0;
  const u8 *ptr = ip_str;
  u8 bytes[4] = {0};

  if (!ip_str || !out_ip)
  {
    return ERR;
  }

  for (i = 0; i < 4; i++)
  {
    if (*ptr < '0' || *ptr > '9')
    {
      return ERR;
    }

    val = 0;
    while (*ptr >= '0' && *ptr <= '9')
    {
      val = val * 10 + (*ptr - '0');
      ptr++;
      if (val > 255)
      {
        return ERR;
      }
    }

    if (i < 3)
    {
      if (*ptr != '.')
      {
        return ERR;
      }
      ptr++;
    }

    bytes[i] = (u8)val;
  }

  if (*ptr != '\0')
  {
    return ERR;
  }

  memcpy(out_ip, bytes, 4);

  return OK;
}

int ip_string(const u8 *ip_addr, u8 *out_str, u32 out_str_len)
{
  u32 i = 0;
  u32 len = 0;
  u8 byte = 0;
  u8 tmp[3];
  u32 digits = 0;
  int j = 0;

  if (!ip_addr || !out_str || out_str_len < (u32)IP_STR_MAX_LEN)
  {
    return ERR;
  }

  for (i = 0; i < 4; i++)
  {
    byte = ip_addr[i];

    digits = 0;
    if (byte == 0)
    {
      out_str[len++] = '0';
    }
    else
    {
      while (byte > 0)
      {
        tmp[digits++] = '0' + (byte % 10);
        byte /= 10;
      }
      for (j = digits - 1; j >= 0; j--)
      {
        out_str[len++] = tmp[j];
      }
    }

    if (i < 3)
    {
      out_str[len++] = '.';
    }
  }

  out_str[len] = '\0';

  return OK;
}

int print_ip_packet(const u8 *pkt, u32 pkt_len)
{
  const ip_hdr *hdr = NULL;
  u32 i = 0;
  u8 src_str[16] = {0};
  u8 dst_str[16] = {0};

  if (!pkt || pkt_len < (u32)sizeof(ip_hdr))
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
  ip_string(hdr->src, src_str, sizeof(src_str));
  printf("src: %s\n", src_str);
  ip_string(hdr->dst, dst_str, sizeof(dst_str));
  printf("dst: %s\n", dst_str);
  printf("checksum: 0x%04x\n", read_be16(hdr->checksum));
  printf("-- IP HEADER --\n");

  printf("-- IP PAYLOAD --\n");
  for (i = (u32)sizeof(ip_hdr); i < pkt_len; i++)
  {
    printf("%02x ", pkt[i]);
  }
  printf("\n");
  printf("-- IP PAYLOAD --\n");

  /* checksum over received header: 0 = valid */
  printf("checksum validation: %d\n", checksum(hdr, (u32)sizeof(ip_hdr)));

  return OK;
}
