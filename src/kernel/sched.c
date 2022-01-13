#include <arch/memory.h>
#include <elf.h>
#include <kernel/sched.h>
#include <lib/lock.h>
#include <lib/print.h>

static size_t current_pid = 0;
static size_t current_tick = 9;
static size_t index = 0;

// QWORD's abi
#define AT_ENTRY 10
#define AT_PHDR 20
#define AT_PHENT 21
#define AT_PHNUM 22
#define AT_RANDOM 25
#define AT_EXECFN 31

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

static uint32_t lock = 0;

Task *sched_tick()
{

    spin_lock(&lock);

    if (current_tick + 1 >= SWITCH_TICK)
    {
        current_tick = 0;

        if ((int)index == processes.length)
            index = 0;

        current_task = processes.data[index++];

        spin_release(&lock);

        return current_task;
    }

    current_tick++;

    spin_release(&lock);
    return current_task;
}

void sched_init(void)
{
    vec_init(&processes);
    current_tick = 9;
}

void sched_push(Task *t)
{

    spin_lock(&lock);

    vec_push(&processes, t);

    spin_release(&lock);
}

void sched_new_elf_process(char *path, const char **argv, const char **envp)
{
    char *ld_path = NULL;
    Auxval val = {};

    Task *t = task_init(0);

    VfsNode *elf_file = vfs_open(path, O_RDWR);

    t->stack.rip = elf_load_program(elf_file->address, 0, t, &val, &ld_path);

    size_t *stack = (size_t *)t->sp;

    // ----- The following code is from mint's lyre, all copyright goes to them (idk if thats how you do it im not a lawyer) ----
    uintptr_t stack_top = (uintptr_t)stack;

    /* Push all strings onto the stack. */
    size_t nenv = 0;
    for (char **elem = (char **)envp; *elem; elem++)
    {
        stack = (void *)stack - (strlen(*elem) + 1);
        strcpy((char *)stack, *elem);
        nenv++;
    }

    size_t nargs = 0;
    for (char **elem = (char **)argv; *elem; elem++)
    {
        stack = (void *)stack - (strlen(*elem) + 1);
        strcpy((char *)stack, *elem);
        nargs++;
    }

    /* Align strp to 16-byte so that the following calculation becomes easier. */
    stack = (void *)stack - ((uintptr_t)stack & 0xf);

    /* Make sure the *final* stack pointer is 16-byte aligned.
            - The auxv takes a multiple of 16-bytes; ignore that.
            - There are 2 markers that each take 8-byte; ignore that, too.
            - Then, there is argc and (nargs + nenv)-many pointers to args/environ.
              Those are what we *really* care about. */
    if ((nargs + nenv + 1) & 1)
        stack--;

    *(--stack) = 0;
    *(--stack) = 0; /* Zero auxiliary vector entry */
    stack -= 2;
    *stack = AT_ENTRY;
    *(stack + 1) = val.at_entry;
    stack -= 2;
    *stack = AT_PHDR;
    *(stack + 1) = val.at_phdr;
    stack -= 2;
    *stack = AT_PHENT;
    *(stack + 1) = val.at_phent;
    stack -= 2;
    *stack = AT_PHNUM;
    *(stack + 1) = val.at_phnum;

    uintptr_t sa = t->stack.rsp;

    *(--stack) = 0; /* Marker for end of environ */
    stack -= nenv;
    for (size_t i = 0; i < nenv; i++)
    {
        sa -= strlen(envp[i]) + 1;
        stack[i] = sa;
    }

    *(--stack) = 0; /* Marker for end of argv */
    stack -= nargs;
    for (size_t i = 0; i < nargs; i++)
    {
        sa -= strlen(argv[i]) + 1;
        stack[i] = sa;
    }
    *(--stack) = nargs; /* argc */

    t->stack.rsp -= (stack_top - (uintptr_t)stack);

    if (ld_path)
    {
        VfsNode *ld_file = vfs_open(ld_path, O_RDWR);

        if (nerd_logs())
        {
            log("ld.so: %s", ld_path);
        }

        if (!ld_file)
        {
            log("ERROR: couldn't find ld.so");
        }

        Auxval ld_val = {};
        char *ld;

        elf_load_program(ld_file->address, 0x40000000, t, &ld_val, &ld);
        t->stack.rip = ld_val.at_entry;

        free(ld_path);
    }

    sched_push(t);
}