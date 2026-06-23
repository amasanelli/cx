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

  *n_recv = 0;

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

/* scan /proc/net/route for the best gateway on a specific interface (longest-prefix-match) */
static int read_gateway(const u8 *iface_name, u8 *out_gateway)
{
  FILE *f = NULL;
  char line[256] = {0};
  char r_iface[IFNAMSIZ] = {0};
  u32 gw = 0;
  u32 flags = 0;
  u32 mask = 0;
  u32 best_mask = 0;
  int n = 0;
  bool found = FALSE;

  memset(out_gateway, 0, IP_ADDR_LEN);

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
    n = sscanf(line, "%15s %*x %x %x %*u %*u %*u %x", r_iface, &gw, &flags, &mask);
    if (n != 4)
    {
      continue;
    }
    if (strcmp(r_iface, (const char *)iface_name) != 0)
    {
      continue;
    }
    if (!(flags & RTF_UP) || !(flags & RTF_GATEWAY))
    {
      continue;
    }
    if (!found || mask > best_mask)
    {
      best_mask = mask;
      out_gateway[0] = (u8)(gw & 0xFF);
      out_gateway[1] = (u8)((gw >> 8) & 0xFF);
      out_gateway[2] = (u8)((gw >> 16) & 0xFF);
      out_gateway[3] = (u8)((gw >> 24) & 0xFF);
      found = TRUE;
    }
  }

  fclose(f);
  return OK; /* gateway stays zero when no RTF_GATEWAY route exists (on-link) */
}

int get_iface_info(const u8 *dst_ip, iface_info *out)
{
  int skt = -1;
  struct sockaddr_in dst = {0};
  struct sockaddr_in src = {0};
  struct sockaddr_ll *ll = NULL;
  socklen_t slen = sizeof(src);
  struct ifaddrs *ifas = NULL;
  struct ifaddrs *ifa = NULL;

  if (!dst_ip || !out)
  {
    return ERR;
  }

  memset(out, 0, sizeof(*out));

  skt = socket(AF_INET, SOCK_DGRAM, 0);
  if (skt < 0)
  {
    return ERR;
  }

  dst.sin_family = AF_INET;
  memcpy(&dst.sin_addr.s_addr, dst_ip, IP_ADDR_LEN);
  write_be16(1, (u8 *)&dst.sin_port); /* non-zero port required; no packet is sent */

  if (connect(skt, (struct sockaddr *)&dst, sizeof(dst)) < 0)
  {
    close(skt);
    return ERR;
  }

  if (getsockname(skt, (struct sockaddr *)&src, &slen) < 0)
  {
    close(skt);
    return ERR;
  }

  if (getifaddrs(&ifas) < 0)
  {
    close(skt);
    return ERR;
  }

  for (ifa = ifas; ifa; ifa = ifa->ifa_next)
  {
    if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
    {
      continue;
    }
    if (((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr != src.sin_addr.s_addr)
    {
      continue;
    }
    break;
  }

  close(skt);

  if (!ifa)
  {
    freeifaddrs(ifas);
    return ERR;
  }

  memcpy(out->ip, &src.sin_addr.s_addr, IP_ADDR_LEN);
  if (ifa->ifa_netmask)
  {
    memcpy(out->netmask, &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr, IP_ADDR_LEN);
  }
  memcpy(out->name, ifa->ifa_name, IFNAMSIZ - 1);

  for (ifa = ifas; ifa; ifa = ifa->ifa_next)
  {
    if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
    {
      continue;
    }
    if (strcmp(ifa->ifa_name, (char *)out->name) != 0)
    {
      continue;
    }
    break;
  }

  if (!ifa)
  {
    freeifaddrs(ifas);
    return ERR;
  }

  ll = (struct sockaddr_ll *)ifa->ifa_addr;
  if (ll->sll_halen < ETH_ADDR_LEN)
  {
    freeifaddrs(ifas);
    return ERR;
  }
  out->index = (u32)ll->sll_ifindex;
  memcpy(out->mac, ll->sll_addr, ETH_ADDR_LEN);

  freeifaddrs(ifas);

  if (read_gateway(out->name, out->gateway) != OK)
  {
    return ERR;
  }

  return OK;
}
