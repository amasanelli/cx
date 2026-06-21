#include "arp.h"

int arp_build_packet(const u8 *src_ip, const u8 *src_mac, const u8 *dst_ip, u8 **out_pkt, u32 *out_pkt_len)
{
  u8 *buf = NULL;
  arp_hdr *hdr = NULL;

  if (!out_pkt || !out_pkt_len || !src_ip || !src_mac || !dst_ip)
  {
    return ERR;
  }

  *out_pkt = NULL;
  *out_pkt_len = (u32)sizeof(arp_hdr);

  buf = (u8 *)malloc(sizeof(arp_hdr));
  if (!buf)
  {
    return ERR;
  }
  memset(buf, 0, sizeof(arp_hdr));

  hdr = (arp_hdr *)buf;

  write_be16(ARP_HW_ETHER, hdr->hw_type);
  write_be16(ARP_PROTO_IP, hdr->proto_type);
  hdr->hw_addr_len = (u8)ETH_ADDR_LEN;
  hdr->proto_addr_len = 4;
  write_be16((u16)ARP_OPCODE_REQUEST, hdr->opcode);

  memcpy(hdr->sender_mac, src_mac, ETH_ADDR_LEN);
  memcpy(hdr->sender_ip, src_ip, 4);
  /* target_mac: stays zero — that's what we're trying to discover */
  memcpy(hdr->target_ip, dst_ip, 4);

  *out_pkt = buf;

  return OK;
}

int print_arp_packet(const u8 *pkt, u32 pkt_len)
{
  const arp_hdr *hdr = NULL;

  if (!pkt || pkt_len < (u32)sizeof(arp_hdr))
  {
    return ERR;
  }

  hdr = (const arp_hdr *)pkt;

  printf("-- ARP HEADER --\n");
  printf("hw_type: 0x%04x\n", read_be16(hdr->hw_type));
  printf("proto_type: 0x%04x\n", read_be16(hdr->proto_type));
  printf("hw_addr_len: %d\n", hdr->hw_addr_len);
  printf("proto_addr_len: %d\n", hdr->proto_addr_len);
  printf("opcode: %d\n", read_be16(hdr->opcode));
  printf("sender_mac: %02x:%02x:%02x:%02x:%02x:%02x\n", hdr->sender_mac[0], hdr->sender_mac[1], hdr->sender_mac[2], hdr->sender_mac[3], hdr->sender_mac[4], hdr->sender_mac[5]);
  printf("sender_ip: %u.%u.%u.%u\n", hdr->sender_ip[0], hdr->sender_ip[1], hdr->sender_ip[2], hdr->sender_ip[3]);
  printf("target_mac: %02x:%02x:%02x:%02x:%02x:%02x\n", hdr->target_mac[0], hdr->target_mac[1], hdr->target_mac[2], hdr->target_mac[3], hdr->target_mac[4], hdr->target_mac[5]);
  printf("target_ip: %u.%u.%u.%u\n", hdr->target_ip[0], hdr->target_ip[1], hdr->target_ip[2], hdr->target_ip[3]);
  printf("-- ARP HEADER --\n");

  return OK;
}
