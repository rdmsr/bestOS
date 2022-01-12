#include "lib/print.h"
#include <arch/asm.h>
#include <arch/devices/com.h>

#define PORT 0x3F8

static int is_transmit_empty()
{
    return asm_in8(PORT + 5) & 0x20;
}

static int serial_received()
{
    return asm_in8(PORT + 5) & 1;
}

void com_initialize()
{
    asm_out8(PORT + 1, 0x00);
    asm_out8(PORT + 3, 0x80);
    asm_out8(PORT + 0, 0x03);
    asm_out8(PORT + 1, 0x00);
    asm_out8(PORT + 3, 0x03);
    asm_out8(PORT + 2, 0xC7);
    asm_out8(PORT + 4, 0x0B);
}
void com_putc(char c)
{
    while (is_transmit_empty() == 0)
        ;
    asm_out8(PORT, c);
}

void com_write_string(char *str)
{
    while (*str)
    {
        com_putc(*str++);
    }
}

char com_getc()
{
    while (serial_received() == 0)
        ;
    return asm_in8(PORT);
}

void host_print(char *str, size_t s)
{
    (void)s;

    com_write_string(str);
}

char host_getc()
{
    return com_getc();
}