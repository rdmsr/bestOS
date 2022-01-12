#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H
#include <lib/base.h>
#include <stivale2.h>

void pmm_free(void *addr, size_t pages);
void pmm_initialize(struct stivale2_struct *stivale);
void *pmm_allocate_zero(size_t pages);
void *pmm_allocate(size_t pages);
#endif