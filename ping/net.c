#include "net.h" /* u8, u16, u32 */

void write_endian16(u8 *dst, u16 value)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  dst[0] = (value >> 8) & 0xFF;
  dst[1] = value & 0xFF;
#else
  dst[0] = value & 0xFF;
  dst[1] = (value >> 8) & 0xFF;
#endif
}

u16 read_endian16(const u8 *src)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  return ((u16)src[0] << 8) | src[1];
#else
  return ((u16)src[1] << 8) | src[0];
#endif
}

void write_endian32(u8 *dst, u32 value)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  dst[0] = (value >> 24) & 0xFF;
  dst[1] = (value >> 16) & 0xFF;
  dst[2] = (value >> 8)  & 0xFF;
  dst[3] =  value        & 0xFF;
#else
  dst[0] =  value        & 0xFF;
  dst[1] = (value >> 8)  & 0xFF;
  dst[2] = (value >> 16) & 0xFF;
  dst[3] = (value >> 24) & 0xFF;
#endif
}

u32 read_endian32(const u8 *src)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  return ((u32)src[0] << 24) | ((u32)src[1] << 16) | ((u32)src[2] << 8) | src[3];
#else
  return ((u32)src[3] << 24) | ((u32)src[2] << 16) | ((u32)src[1] << 8) | src[0];
#endif
}

u16 checksum(const void *buf, u32 len)
{
  const u8 *ptr = (const u8 *)buf;
  u32 sum = 0;

  while (len > 1)
  {
    sum += ((u16)ptr[0] << 8) | ptr[1];
    ptr += 2;
    len -= 2;
  }

  if (len == 1)
  {
    sum += ((u16)ptr[0] << 8);
  }

  while (sum >> 16)
  {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return (u16)(~sum);
}