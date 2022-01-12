#include <lib/abi.h>
#include <lib/errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool nerd_logs();

#define MAYBE_UNUSED __attribute__((unused))
#define PACKED __attribute__((packed))
#define UNUSED(x) (void)x
#define ARRLEN(x) sizeof(x) / sizeof(*x)

#define foreach(NAME, RANGE) for (size_t NAME = 0; NAME < RANGE; NAME += 1)

#define foreachr(NAME, RANGE, ADD) for (size_t NAME = 0; NAME < RANGE; NAME += ADD)

#define ALIGN_UP(NUM, WHAT) (((NUM) + WHAT - 1) & ~(WHAT - 1))
#define ALIGN_DOWN(NUM, WHAT) ((NUM) & ~(WHAT - 1))

#define DIV_ROUNDUP(A, B) ({ \
    (A + (B - 1)) / B;       \
})
