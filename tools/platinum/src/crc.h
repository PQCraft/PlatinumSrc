#ifndef PSRC_COMMON_CRC_H
#define PSRC_COMMON_CRC_H

#include <stdint.h>
#include <stddef.h>

uint32_t crc32(const void*, size_t);
uint64_t crc64(const void*, size_t);
uint32_t strcrc32(const char*);
uint64_t strcrc64(const char*);
uint32_t strcasecrc32(const char*);
uint64_t strcasecrc64(const char*);

#endif
