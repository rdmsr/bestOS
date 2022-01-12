#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H
#include <lib/base.h>
#include <stivale2.h>

void vmm_initialize(struct stivale2_struct *boot);

void vmm_map(uint64_t *pagemap, uintptr_t phys, uintptr_t virt, uint64_t flags);

void vmm_unmap(uint64_t *pagemap, uintptr_t virt, uint64_t flags);

void vmm_load_pagemap(uint64_t *pagemap);

uint64_t *vmm_get_kernel_pagemap();

#endif