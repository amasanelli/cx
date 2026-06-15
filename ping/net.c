#include "net.h" /* u8, u16, u32 */

/*
No platform endianness check needed: shifts operate on the abstract integer
value, not on its memory layout, so MSB is always extracted/inserted first
regardless of host byte order.
*/

void write_be16(u8 *dst, u16 value)
{
  dst[0] = (value >> 8) & 0xFF;
  dst[1] = value & 0xFF;
}

u16 read_be16(const u8 *src)
{
  return ((u16)src[0] << 8) | src[1];
}

void write_be32(u8 *dst, u32 value)
{
  dst[0] = (value >> 24) & 0xFF;
  dst[1] = (value >> 16) & 0xFF;
  dst[2] = (value >> 8) & 0xFF;
  dst[3] = value & 0xFF;
}

u32 read_be32(const u8 *src)
{
  return ((u32)src[0] << 24) | ((u32)src[1] << 16) | ((u32)src[2] << 8) | src[3];
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