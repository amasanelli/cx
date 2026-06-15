#ifndef NET_H
#define NET_H

#include "types.h" /* u8, u16, u32 */

void write_endian16(u8 *dst, u16 value);
u16  read_endian16(const u8 *src);

void write_endian32(u8 *dst, u32 value);
u32  read_endian32(const u8 *src);

u16 checksum(const void *buf, u32 len);

#endif