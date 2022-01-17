#include "arch/memory/pmm.h"
#include "fs/vfs.h"
#include "lib/vec.h"
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

#define STACK_SIZE 0x4000

Task *current_task;

vec_t(Task *) processes;

Task *task_init(uintptr_t entry_point)
{
    Task *new = malloc(sizeof(Task));

    vec_init(&new->descriptors);

    new->pid = current_pid++;

    uintptr_t *stack = pmm_allocate_zero(STACK_SIZE / PAGE_SIZE);

    asm volatile(" fxsave %0 " ::"m"(new->fpu_storage));

    new->sp = stack;
    new->cwd = vfs_get_root();
    new->current_fd = 0;

    new->pagemap = pmm_allocate_zero(1) + MMAP_KERNEL_BASE;

    uint64_t *prev_pagemap = (uint64_t *)((uintptr_t)vmm_get_kernel_pagemap() + MMAP_KERNEL_BASE);

    for (int i = 256; i < 512; i++)
    {
        new->pagemap[i] = prev_pagemap[i];
    }

    // Map task's stack
    for (size_t i = 0; i < (STACK_SIZE / 4096); i++)
    {
        uint64_t phys_addr = i * PAGE_SIZE + (uintptr_t)stack;

        uint64_t virt_addr = i * PAGE_SIZE + (USER_STACK_TOP - STACK_SIZE);

        vmm_map(new->pagemap, phys_addr, virt_addr, 0b111);
    }

    new->stack.rsp = USER_STACK_TOP;
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
    while (processes.length == 0)
    {
        asm_hlt();
    }

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
    current_tick = SWITCH_TICK - 1;
}

void sched_push(Task *t)
{

    spin_lock(&lock);

    vec_push(&processes, t);

    spin_release(&lock);
}

void sched_remove(Task *t)
{
    spin_lock(&lock);
    vec_remove(&processes, t);
    spin_release(&lock);
}

Task *sched_current_task()
{
    return current_task;
}

void sched_new_elf_process(char *path, const char **argv, const char **envp, char *stdin, char *stdout, char *stderr)
{
    char *ld_path = NULL;
    Auxval val = {};

    Task *t = task_init(0);

    VfsNode *elf_file = vfs_open(vfs_get_root(), path, O_RDWR);

    void *elf_buffer = malloc(elf_file->stat.st_size);

    vfs_read(elf_file, 0, elf_file->stat.st_size, elf_buffer);

    t->stack.rip = elf_load_program((uintptr_t)elf_buffer, 0, t, &val, &ld_path);

    if (stdin && stdout && stderr)
    {
        VfsNode *stdin_node = vfs_open(vfs_get_root(), stdin, O_RDWR);
        VfsNode *stdout_node = vfs_open(vfs_get_root(), stdout, O_RDWR);
        VfsNode *stderr_node = vfs_open(vfs_get_root(), stderr, O_RDWR);

        if (!stdin_node || !stdout_node || !stderr_node)
        {
            log("ERROR: couldn't open stdio files");
        }

        FileDescriptor stdin_fd = {.file = stdin_node, .fd = 0};
        FileDescriptor stdout_fd = {.file = stdout_node, .fd = 1};
        FileDescriptor stderr_fd = {.file = stderr_node, .fd = 2};

        vec_push(&t->descriptors, stdin_fd);
        vec_push(&t->descriptors, stdout_fd);
        vec_push(&t->descriptors, stderr_fd);

        t->current_fd = 3;
    }

    size_t *stack = (size_t *)((uintptr_t)t->sp + STACK_SIZE + MMAP_IO_BASE);

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

    // clang-format off
    *(--stack) = 0; *(--stack) = 0; /* Zero auxiliary vector entry */
    stack -= 2; *stack = AT_ENTRY; *(stack + 1) = val.at_entry;
    stack -= 2; *stack = AT_PHDR; *(stack + 1) = val.at_phdr;
    stack -= 2; *stack = AT_PHENT; *(stack + 1) = val.at_phent;
    stack -= 2; *stack = AT_PHNUM; *(stack + 1) = val.at_phnum;
    // clang-format on

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

    size_t skip = (stack_top - (uintptr_t)stack);

    t->stack.rsp -= skip;

    if (ld_path)
    {
        VfsNode *ld_file = vfs_open(vfs_get_root(), ld_path, O_RDWR);

        void *ld_file_buf = malloc(ld_file->stat.st_size);

        vfs_read(ld_file, 0, ld_file->stat.st_size, ld_file_buf);

        if (nerd_logs())
        {
            log("ld.so: %s", ld_path);
        }

        if (!ld_file)
        {
            log("ERROR: couldn't find ld.so");
            while (1)
                ;
        }

        Auxval ld_val = {};

        elf_load_program((uintptr_t)ld_file_buf, 0x40000000, t, &ld_val, NULL);

        free(ld_path);
        free(ld_file_buf);

        t->stack.rip = ld_val.at_entry;
    }

    free(elf_buffer);

    sched_push(t);
}