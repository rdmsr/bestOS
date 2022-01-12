#include "arch/memory/vmm.h"
#include "stivale2.h"
#include <arch/arch.h>
#include <arch/memory.h>
#include <lib/lock.h>
#include <lib/print.h>

#define PML_ENTRY(addr, offset) (size_t)(addr & ((uintptr_t)0x1ff << offset)) >> offset;

uintptr_t *kernel_pagemap = 0;
Lock lock;

static uint64_t *get_next_level(uint64_t *table, size_t index, uint64_t flags)
{
    if (!(table[index] & 1))
    {
        table[index] = (uint64_t)pmm_allocate_zero(1);
        table[index] |= flags;
    }

    return (uint64_t *)((table[index] & ~(0x1ff)) + MMAP_IO_BASE);
}

uint64_t *vmm_get_kernel_pagemap()
{
    return kernel_pagemap;
}

void flush_tlb(void *addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr));
}

void vmm_map(uint64_t *pagemap, uintptr_t physical_address, uintptr_t virtual_address, uint64_t flags)
{
    SPINLOCK_ACQUIRE(lock);
    size_t level4 = PML_ENTRY(virtual_address, 39);
    size_t level3 = PML_ENTRY(virtual_address, 30);
    size_t level2 = PML_ENTRY(virtual_address, 21);
    size_t level1 = PML_ENTRY(virtual_address, 12);

    uint64_t *pml3 = get_next_level(pagemap, level4, flags);
    uint64_t *pml2 = get_next_level(pml3, level3, flags);
    uint64_t *pml1 = get_next_level(pml2, level2, flags);

    pml1[level1] = physical_address | flags;

    flush_tlb((void *)virtual_address);
    LOCK_RELEASE(lock);
}

void vmm_unmap(uint64_t *pagemap, uintptr_t virtual_address, uint64_t flags)
{
    size_t level4 = PML_ENTRY(virtual_address, 39);
    size_t level3 = PML_ENTRY(virtual_address, 30);
    size_t level2 = PML_ENTRY(virtual_address, 21);
    size_t level1 = PML_ENTRY(virtual_address, 12);

    uint64_t *pml3 = get_next_level(pagemap, level4, flags);
    uint64_t *pml2 = get_next_level(pml3, level3, flags);
    uint64_t *pml1 = get_next_level(pml2, level2, flags);

    pml1[level1] = 0;
}

void vmm_map_range(uint64_t *pagemap, uint64_t start, uint64_t end, uint64_t offset, uint64_t flags)
{
    for (uintptr_t i = start; i < end; i += PAGE_SIZE)
    {
        vmm_map(pagemap, i, i + offset, flags);
    }
}

void vmm_load_pagemap(uint64_t *pagemap)
{
    asm_write_cr3((uint64_t)pagemap);
}

void vmm_initialize(struct stivale2_struct *stivale2_struct)
{

    struct stivale2_struct_tag_memmap *memory_map = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    kernel_pagemap = (uint64_t *)pmm_allocate_zero(1);

    vmm_map_range(kernel_pagemap, 0, 0x8000000, MMAP_KERNEL_BASE, 0b11);
    vmm_map_range(kernel_pagemap, 0, 0x100000000, MMAP_IO_BASE, 0b11);

    foreach (i, memory_map->entries)
    {
        struct stivale2_mmap_entry entry = memory_map->memmap[i];

        if (entry.type == STIVALE2_MMAP_USABLE)
        {
            foreachr(p, entry.length, PAGE_SIZE)
            {
                vmm_map(kernel_pagemap, p, p + MMAP_IO_BASE, 0b11);
            }
        }
    }

    vmm_load_pagemap(kernel_pagemap);
}