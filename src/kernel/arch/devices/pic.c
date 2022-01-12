#include <arch/asm.h>
#include <arch/devices/pic.h>
#include <lib/base.h>

void pic_remap(void)
{
    asm_out8(0x20, 0x11);
    asm_out8(0xA0, 0x11);
    asm_out8(0x21, 0x20);
    asm_out8(0xA1, 0x28);
    asm_out8(0x21, 0x04);
    asm_out8(0xA1, 0x02);
    asm_out8(0x21, 0x01);
    asm_out8(0xA1, 0x01);
    asm_out8(0x21, 0x0);
    asm_out8(0xA1, 0x0);
}

void pic_initialize(void)
{
    pic_remap();

    int divisor = 1193180 / 1000;

    asm_out8(0x43, 0x36);
    asm_out8(0x40, divisor & 0xff);
    asm_out8(0x40, (divisor >> 8) & 0xFF);
}

void pic_eoi(int intno)
{
    if (intno >= 40)
    {
        asm_out8(0xA0, 0x20);
    }

    asm_out8(0x20, 0x20);
}
