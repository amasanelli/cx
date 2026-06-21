#ifndef NET_H
#define NET_H

#include "types.h" /* u8, u16, u32 */

void write_be16(u16 value, u8 *out);
u16 read_be16(const u8 *src);

void write_be32(u32 value, u8 *out);
u32 read_be32(const u8 *src);

u16 checksum(const void *buf, u32 len);

#endif
