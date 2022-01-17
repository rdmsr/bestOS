#include "fs/tmpfs.h"
#include "fs/vfs.h"
#include <fs/ustar.h>
#include <lib/alloc.h>
#include <lib/mem.h>
#include <lib/print.h>
#include <lib/str.h>

static uint64_t addr = 0;

struct tar_header
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

unsigned int getsize(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;
}

uintptr_t create_tmpfs_files(uintptr_t address)
{
    size_t size = 0;

    size_t i;

    for (i = 0;; i++)
    {
        struct tar_header *header = (struct tar_header *)address;

        if (header->name[0] == '\0')
            break;

        if (header->typeflag[0] == '5')
        {
            char *name = strdup(header->name);

            name[strlen(name) - 1] = '\0';

            char *new_name = malloc(strlen(name) + 2);

            new_name[0] = '/';

            memcpy(new_name + 1, name, strlen(name));

            vfs_mkdir(new_name);
        }

        else
        {
            char *new_name = malloc(strlen(header->name) + 2);

            new_name[0] = '/';

            memcpy(new_name + 1, header->name, strlen(header->name));

            tmpfs_create_file(new_name, (uintptr_t)header + 512, getsize(header->size));
        }

        size = getsize(header->size);

        address += ((size / 512) + 1) * 512;

        if (size % 512)
        {
            address += 512;
        }
    }

    return 0xc001c0de;
}

void ustar_initialize(uint64_t _addr)
{
    addr = _addr;
    create_tmpfs_files(addr);
}

int ustar_read(VfsNode *node, size_t offset, size_t count, void *buffer)
{
    memcpy(buffer, (void *)(node->address + offset), count);

    return count;
}

int ustar_write(VfsNode *node, size_t offset, size_t count, void *buffer)
{
    UNUSED(node);
    UNUSED(offset);
    UNUSED(count);
    UNUSED(buffer);

    log("USTAR can't write.");
    return -1;
}

Filesystem ustar_fs()
{
    Filesystem fs = {.read = ustar_read, .name = "ustar"};

    return fs;
}
