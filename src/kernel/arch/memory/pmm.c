#include <arch/arch.h>
#include <arch/memory.h>
#include <lib/mem.h>
#include <lib/print.h>
#include <stivale2.h>

#define BIT_SET(BIT, MAP) (MAP[(BIT) / 8] |= (1 << ((BIT) % 8)))
#define BIT_CLEAR(BIT, MAP) (MAP[(BIT) / 8] &= ~(1 << ((BIT) % 8)))
#define BIT_TEST(BIT, MAP) ((MAP[(BIT) / 8] >> ((BIT) % 8)) & 1)

static uintptr_t highest_page = 0;
static size_t usable_pages = 0;

struct
{
    uint8_t *data;
    size_t size;
} bitmap;

void clear_page(void *addr)
{
    BIT_CLEAR((uint64_t)addr / PAGE_SIZE, bitmap.data);
    usable_pages++;
}

void set_page(void *addr)
{
    BIT_SET((uint64_t)addr / PAGE_SIZE, bitmap.data);
    usable_pages--;
}

void set_pages(void *addr, size_t page_count)
{
    size_t i;
    for (i = 0; i < page_count; i++)
    {
        set_page((void *)(addr + (i * PAGE_SIZE)));
    }
}

void pmm_free(void *addr, size_t pages)
{
    size_t i;
    for (i = 0; i < pages; i++)
    {
        clear_page((void *)(addr + (i * PAGE_SIZE)));
    }
}

void *pmm_allocate(size_t pages)
{
    if (pages > bitmap.size)
    {
        log("pmm_allocate(%d): allocation too big", bitmap.size);
        return NULL;
    }

    for (size_t i = 0; i < highest_page / PAGE_SIZE; i++)
    {
        for (size_t j = 0; j < pages; j++)
        {
            if (BIT_TEST(i, bitmap.data))
            {
                break;
            }

            else if (j == pages - 1)
            {
                void *ret = (void *)(i * PAGE_SIZE);

                set_pages(ret, pages);

                return ret;
            }
        }
    }

    return NULL;
}

void *pmm_allocate_zero(size_t pages)
{
    void *ret = pmm_allocate(pages);

    memset(ret + MMAP_IO_BASE, 0, pages * PAGE_SIZE);

    return ret;
}

void pmm_initialize(struct stivale2_struct *bootinfo)
{
    uintptr_t top = 0;

    struct stivale2_struct_tag_memmap *memory_map = stivale2_get_tag(bootinfo, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    foreach (i, memory_map->entries)
    {
        struct stivale2_mmap_entry *entry = &memory_map->memmap[i];

        if (entry->type != STIVALE2_MMAP_USABLE && entry->type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE && entry->type != STIVALE2_MMAP_KERNEL_AND_MODULES)
        {
            continue;
        }

        top = entry->base + entry->length;

        if (top > highest_page)
        {
            highest_page = top;
        }
    }

    size_t bitmap_size = ALIGN_UP(ALIGN_DOWN(highest_page, PAGE_SIZE) / PAGE_SIZE / 8, PAGE_SIZE);

    if (bitmap_size == 0)
    {
        log("PANIC bitmap_size == 0");

        while (1)
        {
            asm_hlt();
        }
    }

    bitmap.size = bitmap_size;

    foreach (i, memory_map->entries)
    {
        struct stivale2_mmap_entry *entry = &memory_map->memmap[i];

        if (entry->type == STIVALE2_MMAP_USABLE && entry->length >= bitmap_size)
        {
            bitmap.data = (uint8_t *)(entry->base + MMAP_IO_BASE);
            entry->base += bitmap_size;
            entry->length -= bitmap_size;
            break;
        }
    }

    memset(bitmap.data, 0xff, bitmap.size);

    foreach (i, memory_map->entries)
    {
        if (memory_map->memmap[i].type == STIVALE2_MMAP_USABLE)
        {
            pmm_free((void *)memory_map->memmap[i].base, memory_map->memmap[i].length / PAGE_SIZE);
        }
    }
}