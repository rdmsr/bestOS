#include <arch/asm.h>
#include <arch/idt.h>

extern void idt_flush(uintptr_t idt_ptr);

static IdtDescriptor idt[256];

static IdtPointer idtr;

extern uintptr_t __interrupt_vector[];

static IdtDescriptor idt_make_entry(uint64_t offset, uint8_t type)
{
    return (IdtDescriptor){
        .offset_lo = offset & 0xFFFF,
        .selector = 0x28,
        .ist = 0,
        .type_attr = type,
        .offset_mid = (offset >> 16) & 0xFFFF,
        .offset_hi = (offset >> 32) & 0xFFFFFFFF,
        .zero = 0};
}

void install_isr(void)
{
    foreach (i, 256)
    {
        idt[i] = idt_make_entry(__interrupt_vector[i], INTGATE);
    }
    idt[66] = idt_make_entry(__interrupt_vector[66], TRAPGATE);
}

void idt_initialize()
{
    idtr.size = (256 * sizeof(IdtDescriptor)) - 1;

    idtr.addr = (uint64_t)idt;

    install_isr();

    idt_flush((uintptr_t)&idtr);

    asm_sti();
}