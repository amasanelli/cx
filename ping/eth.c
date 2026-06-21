#include "eth.h"

int eth_build_packet(const u8 *dst_mac, const u8 *src_mac, u16 ethertype, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  u8 *buf = NULL;
  eth_hdr *hdr = NULL;

  if (!out_pkt || !out_pkt_len || !dst_mac || !src_mac)
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

  *out_pkt = NULL;
  *out_pkt_len = (u32)sizeof(eth_hdr) + pld_len;

  buf = (u8 *)malloc(*out_pkt_len);
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, *out_pkt_len);

  hdr = (eth_hdr *)buf;

  memcpy(hdr->dst, dst_mac, ETH_ADDR_LEN);    /* destination MAC (caller-provided) */
  memcpy(hdr->src, src_mac, ETH_ADDR_LEN);    /* source MAC (our NIC's MAC) */
  write_be16(ethertype, hdr->ethertype);  /* payload protocol type (passed in) */

  if (pld_len > 0)
  {
    memcpy(buf + sizeof(eth_hdr), pld, pld_len);
  }

  *out_pkt = buf;

  return OK;
}

int build_eth_ip_packet(const u8 *dst_mac, const u8 *src_mac, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  return eth_build_packet(dst_mac, src_mac, ETHER_TYPE_IP, pld, pld_len, out_pkt, out_pkt_len);
}

int build_eth_arp_packet(const u8 *dst_mac, const u8 *src_mac, const u8 *pld, u32 pld_len, u8 **out_pkt, u32 *out_pkt_len)
{
  return eth_build_packet(dst_mac, src_mac, ETHER_TYPE_ARP, pld, pld_len, out_pkt, out_pkt_len);
}

int print_eth_packet(const u8 *pkt, u32 pkt_len)
{
  const eth_hdr *hdr = NULL;
  u32 i = 0;

  if (!pkt || pkt_len < (u32)sizeof(eth_hdr))
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
  for (i = (u32)sizeof(eth_hdr); i < pkt_len; i++)
  {
    printf("%02x ", pkt[i]);
  }
  printf("\n");
  printf("-- ETH PAYLOAD --\n");

  return OK;
}
