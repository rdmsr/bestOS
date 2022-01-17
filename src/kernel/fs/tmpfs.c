#include "fs/vfs.h"
#include "lib/mem.h"
#include <fs/tmpfs.h>
#include <lib/print.h>

void tmpfs_create_file(char *name, uintptr_t address, size_t size)
{
    VfsNode *ret = malloc(sizeof(VfsNode));

    ret->name = strdup(basename(name));

    ret->type = VFS_FILE;

    ret->parent = vfs_get_parent(vfs_get_root(), name);

    ret->mountpoint = ret->parent->mountpoint;

    ret->stat.st_size = size;

    void *new_addr = malloc(size + 1);

    ret->address = (uintptr_t)new_addr;

    ret->internal_address = address;

    vec_push(&ret->parent->children, ret);
}

int tmpfs_read(VfsNode *node, size_t offset, size_t count, void *buffer)
{
    if (!node->init)
    {
        memcpy((void *)node->address, (void *)node->internal_address, node->stat.st_size);

        ((char *)node->address)[node->stat.st_size - 1] = -1;

        node->init = true;
    }

    if (!buffer)
        return -1;

    memcpy(buffer, (void *)(node->address + offset), count);

    node->cursor += count;

    return count;
}

int tmpfs_write(VfsNode *node, size_t count, void *buffer)
{

    if (node->o_mode & O_APPEND)
    {
        node->cursor = node->stat.st_size;

        node->address = (uintptr_t)realloc((void *)node->address, node->stat.st_size + count + 1);

        memcpy((void *)node->address + node->cursor, buffer, count);

        node->stat.st_size += count;

        return 0;
    }

    else
    {
        if ((int)node->cursor >= node->stat.st_size)
        {
            node->address = (uintptr_t)realloc((void *)node->address, node->stat.st_size + count + 1);
        }

        memcpy((void *)node->address + node->cursor, buffer, count);

        node->stat.st_size += count;
        node->cursor += count;
    }

    return count;
}

Filesystem tmpfs()
{
    return (Filesystem){.read = tmpfs_read, .write = tmpfs_write, .name = "tmpfs"};
}
