#ifndef KERNEL_USTAR_H
#define KERNEL_USTAR_H
#include <fs/vfs.h>

void ustar_initialize(uint64_t addr);

Filesystem ustar_fs();

#endif