#include "socket.h"

int open_raw_eth_socket(int *skt)
{
  u8 proto[2] = {0};

  if (!skt)
  {
    return ERR;
  }

  /* ETH_P_ALL: capture every Ethernet frame type (passed in network byte order) */
  write_be16(proto, (u16)ETH_P_ALL);

  *skt = -1;
  *skt = socket(AF_PACKET, SOCK_RAW, (int)(*(const u16 *)proto));

  if (*skt < 0)
  {
    return ERR;
  }

  return OK;
}

int build_socket_address(int ifindex, skt_addr *addr)
{
  if (!addr)
  {
    return ERR;
  }

  memset(addr, 0, sizeof(skt_addr));

  addr->family = AF_PACKET;                         /* raw link-layer: gives access to full Ethernet frames */
  write_be16((u8 *)&addr->protocol, ETHER_TYPE_IP); /* we are sending IPv4 packets */
  addr->ifindex = ifindex;                          /* NIC to send/recv on */
  /* hatype  (hw addr type)   — kernel fills on recv, ignored by sendto */
  /* pkttype (packet dir)     — kernel fills on recv, ignored by sendto */
  /* halen   (hw addr length) — not needed: dst MAC already in pre-built frame */
  /* addr    (hw addr)        — not needed: dst MAC already in pre-built frame */

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

int get_iface_index(const char *iface, int *ifindex)
{
  if (!iface || !ifindex)
  {
    return ERR;
  }

  *ifindex = (int)if_nametoindex(iface);

  if (*ifindex == 0)
  {
    return ERR;
  }

  return OK;
}

int get_iface_ip(int skt, const char *iface, u32 *ip)
{
  struct ifreq ifr;

  if (!iface || !ip)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFADDR, &ifr) < 0)
  {
    return ERR;
  }

  /* sin_addr.s_addr is NBO; read_be32 converts to host byte order so write_be32 encodes it correctly */
  *ip = read_be32((const u8 *)&((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);

  return OK;
}

int get_iface_mac(int skt, const char *iface, u8 *mac)
{
  struct ifreq ifr;

  if (!iface || !mac)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFHWADDR, &ifr) < 0)
  {
    return ERR;
  }

  memcpy(mac, ifr.ifr_hwaddr.sa_data, ETH_ADDR_LEN);

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