#include "socket.h"

int open_raw_eth_socket(int *out_skt)
{
  u8 proto[2] = {0};

  if (!out_skt)
  {
    return ERR;
  }

  /* ETH_P_ALL: capture every Ethernet frame type (passed in network byte order) */
  write_be16((u16)ETH_P_ALL, proto);

  *out_skt = -1;
  *out_skt = socket(AF_PACKET, SOCK_RAW, (int)(*(const u16 *)proto));

  if (*out_skt < 0)
  {
    return ERR;
  }

  return OK;
}

int build_socket_address(u32 if_i, skt_addr *out_addr)
{
  if (!out_addr)
  {
    return ERR;
  }

  memset(out_addr, 0, sizeof(skt_addr));

  out_addr->family = AF_PACKET;                         /* raw link-layer: gives access to full Ethernet frames */
  write_be16(ETHER_TYPE_IP, (u8 *)&out_addr->protocol); /* we are sending IPv4 packets */
  out_addr->if_i = if_i;                                /* NIC to send/recv on */
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

int get_iface_index(int skt, const u8 *iface, u32 *out_if_i)
{
  struct ifreq ifr = {0};

  if (!iface || !out_if_i)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, (const char *)iface, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFINDEX, &ifr) < 0)
  {
    return ERR;
  }

  *out_if_i = (u32)ifr.ifr_ifindex;

  return OK;
}

int get_iface_ip(int skt, const u8 *iface, u8 *out_ip)
{
  struct ifreq ifr = {0};

  if (!iface || !out_ip)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, (const char *)iface, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFADDR, &ifr) < 0)
  {
    return ERR;
  }

  memcpy(out_ip, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, 4);

  return OK;
}

int get_iface_mac(int skt, const u8 *iface, u8 *out_mac)
{
  struct ifreq ifr = {0};

  if (!iface || !out_mac)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, (const char *)iface, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFHWADDR, &ifr) < 0)
  {
    return ERR;
  }

  memcpy(out_mac, ifr.ifr_hwaddr.sa_data, ETH_ADDR_LEN);

  return OK;
}

int receive_packet(int skt, u8 *buf, u32 buff_len, u32 *n_recv)
{
  ssize_t n = -1;

  if (!n_recv)
  {
    return ERR;
  }

  n = recvfrom(skt, buf, buff_len, 0, NULL, NULL);

  if (n < 0)
  {
    return ERR;
  }

  *n_recv = (u32)n;

  return OK;
}

int set_recv_timeout(int skt, u32 seconds)
{
  struct timeval tv = {0};

  tv.tv_sec = (time_t)seconds;
  tv.tv_usec = 0;

  if (setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
  {
    return ERR;
  }

  return OK;
}

int get_iface_netmask(int skt, const u8 *iface, u8 *out_msk)
{
  struct ifreq ifr = {0};

  if (!iface || !out_msk)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, (const char *)iface, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFNETMASK, &ifr) < 0)
  {
    return ERR;
  }

  memcpy(out_msk, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, 4);

  return OK;
}

int get_iface_gateway(const u8 *iface, u8 *out_gw)
{
  FILE *f = NULL;
  char line[256] = {0};
  char r_iface[IFNAMSIZ] = {0};
  u32 dest = 0;
  u32 r_gw = 0;
  u32 flags = 0;
  int n = 0;

  if (!iface || !out_gw)
  {
    return ERR;
  }

  f = fopen("/proc/net/route", "r");
  if (!f)
  {
    return ERR;
  }

  /* skip header line */
  if (!fgets(line, (int)sizeof(line), f))
  {
    fclose(f);
    return ERR;
  }

  while (fgets(line, (int)sizeof(line), f))
  {
    /* columns: Iface Dest GW Flags RefCnt Use Metric Mask MTU Window IRTT */
    n = sscanf(line, "%15s %x %x %x", r_iface, &dest, &r_gw, &flags);

    if (n != 4)
    {
      continue;
    }

    if (strncmp(r_iface, (const char *)iface, 15) != 0)
    {
      continue;
    }

    if ((flags & (RTF_UP | RTF_GATEWAY)) != (RTF_UP | RTF_GATEWAY))
    {
      continue;
    }

    if (dest != 0)
    {
      continue;
    }

    /* /proc/net/route stores gateway as LE u32 in hex; extract octets in order */
    out_gw[0] = (u8)(r_gw & 0xFF);
    out_gw[1] = (u8)((r_gw >> 8) & 0xFF);
    out_gw[2] = (u8)((r_gw >> 16) & 0xFF);
    out_gw[3] = (u8)((r_gw >> 24) & 0xFF);

    fclose(f);
    return OK;
  }

  fclose(f);
  return ERR;
}

int get_iface_for_ip(const u8 *dst_ip, u8 *out_iface)
{
  FILE *f = NULL;
  char line[256] = {0};
  char r_iface[IFNAMSIZ] = {0};
  u32 dest = 0;
  u32 r_gw = 0;
  u32 flags = 0;
  u32 mask = 0;
  int n = 0;
  u32 best_mask = 0;
  bool found = FALSE;
  u32 dst_le = 0;

  if (!dst_ip || !out_iface)
  {
    return ERR;
  }

  /* dst_ip is network-byte-order bytes; /proc/net/route stores values as LE u32 */
  dst_le = (u32)dst_ip[0] | ((u32)dst_ip[1] << 8) | ((u32)dst_ip[2] << 16) | ((u32)dst_ip[3] << 24);

  f = fopen("/proc/net/route", "r");
  if (!f)
  {
    return ERR;
  }

  /* skip header line */
  if (!fgets(line, (int)sizeof(line), f))
  {
    fclose(f);
    return ERR;
  }

  while (fgets(line, (int)sizeof(line), f))
  {
    /* columns: Iface Dest GW Flags RefCnt Use Metric Mask MTU Window IRTT */
    n = sscanf(line, "%15s %x %x %x %*u %*u %*u %x", r_iface, &dest, &r_gw, &flags, &mask);
    if (n != 5)
    {
      continue;
    }
    if (!(flags & RTF_UP))
    {
      continue;
    }
    if ((dst_le & mask) != dest)
    {
      continue;
    }
    if (!found || mask > best_mask)
    {
      best_mask = mask;
      strncpy((char *)out_iface, r_iface, IFNAMSIZ - 1);
      out_iface[IFNAMSIZ - 1] = '\0';
      found = TRUE;
    }
  }

  fclose(f);

  return found ? OK : ERR;
}
