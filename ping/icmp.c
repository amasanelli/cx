#include <stdlib.h> /* malloc */
#include <string.h> /* memset, memcpy */
#include "net.h"    /* write_be16, checksum */
#include "icmp.h"   /* icmp_hdr, ICMP_*, u8, u16, u32 */

int icmp_build_packet(u8 type, u8 code, u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  u8 *buf = NULL;
  icmp_hdr *hdr = NULL;

  if (!pkt || !pkt_len)
  {
    return ERR;
  }

  if (pld_len > 0 && (!pld || pld_len > ICMP_MAX_PLD_SIZE))
  {
    return ERR;
  }

  *pkt = NULL;
  *pkt_len = ICMP_HDR_SIZE + pld_len;

  buf = (u8 *)malloc(*pkt_len);
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, *pkt_len);

  hdr = (icmp_hdr *)buf;

  hdr->type = type;
  hdr->code = code;
  /* checksum = 0 */
  write_be16(hdr->id, id);
  write_be16(hdr->seq, seq);

  if (pld_len > 0)
  {
    memcpy(buf + ICMP_HDR_SIZE, pld, pld_len);
  }

  write_be16(hdr->checksum, checksum(buf, *pkt_len));

  *pkt = buf;

  return OK;
}

int build_ping_packet(u16 id, u16 seq, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  return icmp_build_packet(ICMP_ECHO_REQUEST, 0, id, seq, pld, pld_len, pkt, pkt_len);
}