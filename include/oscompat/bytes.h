#ifndef OSCOMPAT_BYTES_H
#define OSCOMPAT_BYTES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reads a 16-bit little-endian value from an unaligned address. */
uint16_t os_load_le16(const void* p);

/* Reads a 32-bit little-endian value from an unaligned address. */
uint32_t os_load_le32(const void* p);

/* Reads a 64-bit little-endian value from an unaligned address. */
uint64_t os_load_le64(const void* p);

/* Reads a 16-bit big-endian value from an unaligned address. */
uint16_t os_load_be16(const void* p);

/* Reads a 32-bit big-endian value from an unaligned address. */
uint32_t os_load_be32(const void* p);

/* Reads a 64-bit big-endian value from an unaligned address. */
uint64_t os_load_be64(const void* p);

/* Writes a 16-bit little-endian value to an unaligned address. */
void os_store_le16(void* p, uint16_t v);

/* Writes a 32-bit little-endian value to an unaligned address. */
void os_store_le32(void* p, uint32_t v);

/* Writes a 64-bit little-endian value to an unaligned address. */
void os_store_le64(void* p, uint64_t v);

/* Writes a 16-bit big-endian value to an unaligned address. */
void os_store_be16(void* p, uint16_t v);

/* Writes a 32-bit big-endian value to an unaligned address. */
void os_store_be32(void* p, uint32_t v);

/* Writes a 64-bit big-endian value to an unaligned address. */
void os_store_be64(void* p, uint64_t v);

#ifdef __cplusplus
}
#endif

#endif
