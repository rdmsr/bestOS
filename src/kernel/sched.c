#include <arch/memory.h>
#include <kernel/sched.h>
#include <lib/lock.h>
#include <lib/print.h>

static size_t current_pid = 0;
static size_t current_tick = 9;
static size_t index = 0;

Task *current_task;

vec_t(Task *) processes;

Task *task_init(uintptr_t entry_point)
{
    Task *new = malloc(sizeof(Task));

    vec_init(&new->descriptors);

    new->pid = current_pid++;

    uintptr_t stack = (uintptr_t)malloc(16384);

    new->sp = stack + 16384;

    new->pagemap = pmm_allocate_zero(1) + MMAP_KERNEL_BASE;

    uint64_t *prev_pagemap = (uint64_t *)((uintptr_t)vmm_get_kernel_pagemap() + MMAP_KERNEL_BASE);

    for (int i = 256; i < 512; i++)
    {
        new->pagemap[i] = prev_pagemap[i];
    }

    for (size_t i = 0; i < (16384 / 4096); i++)
    {

        uint64_t phys_addr = i * PAGE_SIZE + ALIGN_DOWN((stack - MMAP_IO_BASE), PAGE_SIZE);
        uint64_t virt_addr = i * PAGE_SIZE + ALIGN_DOWN((USER_STACK_BASE - 16384), PAGE_SIZE);

        vmm_map(new->pagemap, phys_addr, virt_addr, 0b111);
    }

    new->stack.rsp = USER_STACK_BASE;
    new->stack.rip = entry_point;
    new->stack.rflags = 0x202;

    new->stack.cs = 0x43;
    new->stack.ss = 0x3b;

    return new;
}

bool started = false;

bool sched_started()
{
    return started;
}

void sched_start()
{
    started = true;
}

Lock lock;

Task *sched_tick()
{

    SPINLOCK_ACQUIRE(lock);

    if (current_tick + 1 >= SWITCH_TICK)
    {
        current_tick = 0;

        if ((int)index == processes.length)
            index = 0;

        current_task = processes.data[index++];

        LOCK_RELEASE(lock);

        return current_task;
    }

    current_tick++;

    LOCK_RELEASE(lock);
    return current_task;
}

void sched_init(void)
{
    vec_init(&processes);
    current_tick = 9;
}

void sched_push(Task *t)
{
    SPINLOCK_ACQUIRE(lock);

    vec_push(&processes, t);

    LOCK_RELEASE(lock);
}