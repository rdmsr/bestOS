#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#define MMAP_IO_BASE ((uintptr_t)0xffff800000000000)
#define MMAP_KERNEL_BASE ((uintptr_t)0xffffffff80000000)

#include <arch/memory/pmm.h>
#include <arch/memory/vmm.h>

#endif