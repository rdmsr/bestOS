#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H
#include <lib/base.h>
#include <stivale2.h>

#define PROT_NONE 0x00
#define PROT_READ 0x01
#define PROT_WRITE 0x02
#define PROT_EXEC 0x04

#define MAP_PRIVATE 0x01
#define MAP_SHARED 0x02
#define MAP_FIXED 0x04
#define MAP_ANON 0x08
#define MAP_ANONYMOUS 0x08
#define ALLOC_BASE 0x1000000

void vmm_initialize(struct stivale2_struct *boot);

void vmm_map(uint64_t *pagemap, uintptr_t phys, uintptr_t virt, uint64_t flags);

void vmm_unmap(uint64_t *pagemap, uintptr_t virt, uint64_t flags);

void vmm_load_pagemap(uint64_t *pagemap);

uint64_t *vmm_get_kernel_pagemap();

void vmm_mmap(uint64_t *pagemap, size_t hint, size_t size, size_t flags, size_t *used_pages, size_t *ret);

#endif