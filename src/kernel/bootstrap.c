#include <lib/base.h>
#include <lib/str.h>
#include <stivale2.h>

static uint8_t stack[8192];

void _start(struct stivale2_struct *stivale2_struct);
void kernel_main(struct stivale2_struct *stivale2_struct, uint8_t *stack, size_t stack_size);

__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    .entry_point = (uintptr_t)_start,

    .stack = (uintptr_t)stack + sizeof(stack),

    .flags = (1 << 1),

    .tags = (uintptr_t)0};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id)
{
    struct stivale2_tag *current_tag = (struct stivale2_tag *)stivale2_struct->tags;

    for (;;)
    {
        if (!current_tag)
        {
            return NULL;
        }

        if (current_tag->identifier == id)
        {
            return current_tag;
        }

        current_tag = (void *)current_tag->next;
    }
}

bool do_nerd_logs = false;

bool nerd_logs()
{
    return do_nerd_logs;
}

static int errno = 0;

void set_errno(int val)
{
    errno = val;
}

int get_errno()
{
    return errno;
}

void _start(struct stivale2_struct *stivale2_struct)
{
    struct stivale2_struct_tag_cmdline *cmdline = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_CMDLINE_ID);

    if (str_eq((char *)cmdline->cmdline, "log"))
    {
        do_nerd_logs = true;
    }

    kernel_main(stivale2_struct, stack, 8192);
}
