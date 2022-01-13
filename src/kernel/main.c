#include "fs/vfs.h"
#include <arch/arch.h>
#include <arch/devices/com.h>
#include <arch/devices/pic.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/memory.h>
#include <elf.h>
#include <fs/tmpfs.h>
#include <fs/ustar.h>
#include <lib/alloc.h>
#include <lib/lock.h>
#include <lib/print.h>
#include <lib/str.h>
#include <sched.h>
#include <stivale2.h>

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id);

static uint32_t lock = 0;

void *liballoc_alloc(int s)
{
    return pmm_allocate_zero(s) + MMAP_IO_BASE;
}

int liballoc_free(void *p, int s)
{
    pmm_free(p - MMAP_IO_BASE, s);
    return 0;
}
int liballoc_lock()
{
    spin_lock(&lock);
    return 0;
}

int liballoc_unlock()
{
    spin_release(&lock);
    return 0;
}

void print_files(VfsNode *node)
{
    for (int i = 0; i < node->children.length; i++)
    {
        log("\t %s: %s", node->children.data[i]->type == VFS_DIRECTORY ? "directory" : "file", node->children.data[i]->name);
    }
}

void kernel_main(struct stivale2_struct *stivale2_struct, uint8_t *stack, size_t stack_size)
{
    com_initialize();

    log("Welcome to bestOS: An agile, cloud-ready, blockchain-powered, web-scale operating system");
    log("Version: Professional, license_key: 0x%x", 0xc001c0de);

    struct stivale2_struct_tag_modules *modules = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);

    gdt_initialize(stack, stack_size);
    pic_initialize();
    idt_initialize();

    pmm_initialize(stivale2_struct);
    vmm_initialize(stivale2_struct);

    vfs_mount("/", tmpfs());

    for (size_t i = 0; i < modules->module_count; i++)
    {
        if (str_eq("ustar  ", (char *)(modules->modules[i].begin + 257)))
        {
            ustar_initialize(modules->modules[i].begin);
        }
    }

    print_files(vfs_get_root());

    sched_init();

    const char *argv[] = {"some arg", "other arg", NULL};
    const char *envp[] = {"some arg", "other arg", NULL};

    sched_new_elf_process("/yes.elf", argv, envp);

    sched_start();

    for (;;)
    {
        asm_hlt();
    }
}
