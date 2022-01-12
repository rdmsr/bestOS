#include <arch/gdt.h>
#include <lib/mem.h>

Gdt gdt = {
    {{0x0000, 0, 0, 0x00, 0x00, 0},       // NULL
     {0xFFFF, 0, 0, 0x9A, 0x00, 0},       // 16 Bit code
     {0xFFFF, 0, 0, 0x92, 0x00, 0},       // 16 Bit data
     {0xFFFF, 0, 0, 0x9A, 0xCF, 0},       // 32 Bit code
     {0xFFFF, 0, 0, 0x92, 0xCF, 0},       // 32 Bit data
     {0x0000, 0, 0, 0x9A, 0x20, 0},       // 64 Bit code
     {0x0000, 0, 0, 0x92, 0x00, 0},       // 64 Bit data
     {0x0000, 0, 0, 0xF2, 0x00, 0},       // User data
     {0x0000, 0, 0, 0xFA, 0x20, 0}},      // User code
    {0x0000, 0, 0, 0x89, 0x00, 0, 0, 0}}; // TSS

GdtPointer gdtr = {};

static Tss tss = {
    .reserved = 0,
    .rsp = {},
    .reserved0 = 0,
    .ist = {},
    .reserved1 = 0,
    .reserved2 = 0,
    .reserved3 = 0,
    .iopb_offset = 0,
};

TssEntry make_tss_entry(uintptr_t tss)
{
    return (TssEntry){
        .length = sizeof(TssEntry),
        .base0 = (tss & 0xffff),
        .base1 = ((tss >> 16) & 0xff),
        .flags1 = 0b10001001,
        .flags2 = 0,
        .base2 = ((tss >> 24) & 0xff),
        .base3 = (tss >> 32),
        .reserved = 0,
    };
}

extern void gdt_update(uint64_t);
extern void tss_update();

void gdt_initialize(uint8_t *stack, size_t size)
{
    gdtr.base = (uint64_t)&gdt;
    gdtr.limit = sizeof(gdt) - 1;

    gdt.tss = make_tss_entry((uintptr_t)&tss);

    memset(&tss, 0, sizeof(tss));

    tss.rsp[0] = (uintptr_t)(stack + size);

    tss.iopb_offset = sizeof(tss);

    gdt_update((uint64_t)&gdtr);
    tss_update();
}