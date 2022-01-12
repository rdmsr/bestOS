#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H
#include <lib/base.h>

typedef struct PACKED
{
    uint16_t limit;
    uintptr_t base;
} GdtPointer;

typedef struct PACKED
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t granularity;
    uint8_t base2;
} GdtDescriptor;

typedef struct
{
    uint16_t length;
    uint16_t base0;
    uint8_t base1;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base2;
    uint32_t base3;
    uint32_t reserved;
} TssEntry;

typedef struct
{
    GdtDescriptor entries[9];
    TssEntry tss;
} Gdt;

typedef struct PACKED
{
    uint32_t reserved;

    uint64_t rsp[3];

    uint64_t reserved0;

    uint64_t ist[7];

    uint32_t reserved1;
    uint32_t reserved2;
    uint16_t reserved3;

    uint16_t iopb_offset;
} Tss;

void gdt_initialize(uint8_t *stack, size_t size);

#endif