#include "icmp.h"

int icmp_build_packet(u8 type, u8 code, u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  u8 *buf = NULL;
  icmp_hdr *hdr = NULL;

  if (!out_pkt || !out_pkt_len)
  {
    return ERR;
  }

  if (pld_len > ICMP_MAX_PLD_SIZE)
  {
    return ERR;
  }

  if (pld_len > 0 && !pld)
  {
    return ERR;
  }

  *out_pkt = NULL;
  *out_pkt_len = (u32)sizeof(icmp_hdr) + pld_len;

  buf = (u8 *)malloc(*out_pkt_len);
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, *out_pkt_len);

  hdr = (icmp_hdr *)buf;

  hdr->type = type; /* message type (passed in) */
  hdr->code = code; /* subtype (passed in, 0 for echo request) */
  /* checksum = 0 before computing */
  write_be16(id, hdr->id);   /* echo session identifier */
  write_be16(seq, hdr->seq); /* sequence number to match reply */

  if (pld_len > 0)
  {
    memcpy(buf + sizeof(icmp_hdr), pld, pld_len);
  }

  write_be16(checksum(buf, *out_pkt_len), hdr->checksum);

  *out_pkt = buf;

  return OK;
}

int build_ping_packet(u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  return icmp_build_packet(ICMP_ECHO_REQUEST, 0, id, seq, pld, pld_len, out_pkt, out_pkt_len);
}

int build_pong_packet(u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  return icmp_build_packet(ICMP_ECHO_REPLY, 0, id, seq, pld, pld_len, out_pkt, out_pkt_len);
}

int print_icmp_packet(const u8 *pkt, u32 pkt_len)
{
  const icmp_hdr *hdr = NULL;
  u32 i = 0;

  if (!pkt || pkt_len < (u32)sizeof(icmp_hdr))
  {
    return ERR;
  }

  hdr = (const icmp_hdr *)pkt;

  printf("-- ICMP HEADER --\n");
  printf("type: %d\n", hdr->type);
  printf("code: %d\n", hdr->code);
  printf("id: %d\n", read_be16(hdr->id));
  printf("seq: %d\n", read_be16(hdr->seq));
  printf("checksum: 0x%04x\n", read_be16(hdr->checksum));
  printf("-- ICMP HEADER --\n");

  printf("-- ICMP PAYLOAD --\n");
  for (i = (u32)sizeof(icmp_hdr); i < pkt_len; i++)
  {
    printf("%02x ", pkt[i]);
  }
  printf("\n");
  printf("-- ICMP PAYLOAD --\n");

  /* checksum over received ICMP packet: 0 = valid */
  printf("checksum validation: %d\n", checksum(pkt, pkt_len));

  return OK;
}
