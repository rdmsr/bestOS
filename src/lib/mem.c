#include <lib/mem.h>

void *memset(void *s, uint8_t c, size_t n)
{
    uint8_t *dest = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        dest[i] = c;
    }

    return s;
}

void *memcpy(void *to, void *from, size_t n)
{
    uint8_t *dest = (uint8_t *)to;
    uint8_t const *src = (uint8_t const *)from;

    for (size_t i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }

    return to;
}