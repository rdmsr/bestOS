#ifndef KERNEL_TMPFS_H
#define KERNEL_TMPFS_H
#include <fs/vfs.h>
#include <lib/base.h>

void tmpfs_create_file(char *name, uintptr_t address, size_t size);

Filesystem tmpfs();

#endif