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

int set_socket_timeouts(int skt, u32 seconds)
{
  struct timeval tv = {0};

  tv.tv_sec = (time_t)seconds;
  tv.tv_usec = 0;

  if (setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
  {
    return ERR;
  }

  if (setsockopt(skt, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
  {
    return ERR;
  }

  return OK;
}

/* single pass through /proc/net/route: resolves iface name, netmask, and gateway for dst_ip */
static int read_route_info(const u8 *dst_ip, u8 *out_name, u8 *out_netmask, u8 *out_gateway)
{
  FILE *f = NULL;
  char line[256] = {0};
  char r_iface[IFNAMSIZ] = {0};
  u32 dest = 0;
  u32 gw = 0;
  u32 flags = 0;
  u32 mask = 0;
  int n = 0;
  u32 dst_le = 0;
  u32 best_mask = 0;
  bool found = FALSE;

  dst_le = (u32)dst_ip[0] | ((u32)dst_ip[1] << 8) | ((u32)dst_ip[2] << 16) | ((u32)dst_ip[3] << 24);

  f = fopen("/proc/net/route", "r");
  if (!f)
  {
    return ERR;
  }

  if (!fgets(line, (int)sizeof(line), f))
  {
    fclose(f);
    return ERR;
  }

  while (fgets(line, (int)sizeof(line), f))
  {
    /* columns: Iface Dest GW Flags RefCnt Use Metric Mask MTU Window IRTT */
    n = sscanf(line, "%15s %x %x %x %*u %*u %*u %x", r_iface, &dest, &gw, &flags, &mask);
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
      memset(out_name, 0, IFNAMSIZ);
      memcpy(out_name, r_iface, IFNAMSIZ - 1);
      out_name[IFNAMSIZ - 1] = '\0';
      /* route table mask is LE u32; extract octets in order */
      memset(out_netmask, 0, IP_ADDR_LEN);
      out_netmask[0] = (u8)(mask & 0xFF);
      out_netmask[1] = (u8)((mask >> 8) & 0xFF);
      out_netmask[2] = (u8)((mask >> 16) & 0xFF);
      out_netmask[3] = (u8)((mask >> 24) & 0xFF);
      /* gateway from this route: non-zero only when RTF_GATEWAY is set */
      memset(out_gateway, 0, IP_ADDR_LEN);
      if (flags & RTF_GATEWAY)
      {
        out_gateway[0] = (u8)(gw & 0xFF);
        out_gateway[1] = (u8)((gw >> 8) & 0xFF);
        out_gateway[2] = (u8)((gw >> 16) & 0xFF);
        out_gateway[3] = (u8)((gw >> 24) & 0xFF);
      }
      found = TRUE;
    }
  }

  fclose(f);

  return found ? OK : ERR;
}

int get_iface_info(int skt, const u8 *ip, iface_info *out)
{
  struct ifreq ifr = {0};

  if (!ip || !out)
  {
    return ERR;
  }

  memset(out, 0, sizeof(*out));

  if (read_route_info(ip, out->name, out->netmask, out->gateway) != OK)
  {
    return ERR;
  }

  memset(&ifr, 0, sizeof(ifr));
  memcpy(ifr.ifr_name, out->name, IFNAMSIZ - 1);

  if (ioctl(skt, SIOCGIFINDEX, &ifr) < 0)
  {
    return ERR;
  }
  out->index = (u32)ifr.ifr_ifindex;

  if (ioctl(skt, SIOCGIFADDR, &ifr) < 0)
  {
    return ERR;
  }
  memcpy(out->ip, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, IP_ADDR_LEN);

  if (ioctl(skt, SIOCGIFHWADDR, &ifr) < 0)
  {
    return ERR;
  }
  memcpy(out->mac, ifr.ifr_hwaddr.sa_data, ETH_ADDR_LEN);

  return OK;
}
