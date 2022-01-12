#ifndef LIB_MEM_H
#define LIB_MEM_H
#include <lib/base.h>

void *memset(void *s, uint8_t c, size_t n);
void *memcpy(void *to, void *from, size_t n);
#endif