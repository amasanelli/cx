#include "eth.h"

int eth_build_packet(const u8 *dst, const u8 *src, u16 ethertype, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  u8 *buf = NULL;
  eth_hdr *hdr = NULL;

  if (!pkt || !pkt_len || !dst || !src)
  {
    return ERR;
  }

  if (pld_len > ETH_MAX_PLD_SIZE)
  {
    return ERR;
  }

  if (pld_len > 0 && !pld)
  {
    return ERR;
  }

  *pkt = NULL;
  *pkt_len = ETH_HDR_SIZE + pld_len;

  buf = (u8 *)malloc(*pkt_len);
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, *pkt_len);

  hdr = (eth_hdr *)buf;

  memcpy(hdr->dst, dst, ETH_ADDR_LEN);   /* destination MAC (caller-provided) */
  memcpy(hdr->src, src, ETH_ADDR_LEN);   /* source MAC (our NIC's MAC) */
  write_be16(hdr->ethertype, ethertype); /* payload protocol type (passed in) */

  if (pld_len > 0)
  {
    memcpy(buf + ETH_HDR_SIZE, pld, pld_len);
  }

  *pkt = buf;

  return OK;
}

int build_eth_ip_packet(const u8 *dst, const u8 *src, const u8 *pld, u32 pld_len, u8 **pkt, u32 *pkt_len)
{
  return eth_build_packet(dst, src, ETHER_TYPE_IP, pld, pld_len, pkt, pkt_len);
}

int print_eth_packet(const u8 *pkt, u32 pkt_len)
{
  const eth_hdr *hdr = NULL;
  u32 i = 0;

  if (!pkt || pkt_len < ETH_HDR_SIZE)
  {
    return ERR;
  }

  hdr = (const eth_hdr *)pkt;

  printf("-- ETH HEADER --\n");
  printf("dst: %02x:%02x:%02x:%02x:%02x:%02x\n", hdr->dst[0], hdr->dst[1], hdr->dst[2], hdr->dst[3], hdr->dst[4], hdr->dst[5]);
  printf("src: %02x:%02x:%02x:%02x:%02x:%02x\n", hdr->src[0], hdr->src[1], hdr->src[2], hdr->src[3], hdr->src[4], hdr->src[5]);
  printf("ethertype: 0x%04x\n", read_be16(hdr->ethertype));
  printf("-- ETH HEADER --\n");

  printf("-- ETH PAYLOAD --\n");
  for (i = ETH_HDR_SIZE; i < pkt_len; i++)
  {
    printf("%02x ", pkt[i]);
  }
  printf("\n");
  printf("-- ETH PAYLOAD --\n");

  return OK;
}
