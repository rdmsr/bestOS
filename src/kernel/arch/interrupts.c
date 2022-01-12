#include <arch/arch.h>
#include <arch/devices/pic.h>
#include <arch/memory.h>
#include <lib/print.h>
#include <sched.h>

Task *prev_task;

uint64_t interrupts_handler(uint64_t rsp)
{
    Stack *stack = (Stack *)rsp;

    if (stack->intno < 32)
    {
        log("\033[1;31mPANIC !!! \033[0mit's not a bug, it's a feature (%x)", stack->intno);

        while (1)
        {
            asm_cli();
            asm_hlt();
        }
    }

    if (stack->intno == 32)
    {
        if (sched_started())
        {
            if (prev_task)
            {
                prev_task->stack = *stack;
            }

            Task *new_task = sched_tick();

            prev_task = new_task;

            *stack = new_task->stack;

            asm_write_cr3((uint64_t)new_task->pagemap - MMAP_KERNEL_BASE);
        }
    }

    if (stack->intno == 0x42)
    {
        log("%s", (char *)stack->rdi);
    }

    pic_eoi(stack->intno);

    return rsp;
}