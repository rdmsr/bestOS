#include "arch/devices/com.h"
#include "fs/vfs.h"
#include <fs/devfs.h>
#include <lib/lock.h>
#include <lib/print.h>

int devfs_read(VfsNode *node, size_t offset, size_t count, void *buffer)
{
    (void)offset;

    if (str_eq(node->name, "tty"))
    {
        char *buf = malloc(count + 1);

        size_t i = 0;
        char curr_char = 0;

        while (curr_char != '\r')
        {
            curr_char = com_getc();

            buf[i++] = curr_char;

            com_putc(curr_char);
        }

        com_putc('\n');

        memcpy(buffer, buf, count);
        free(buf);

        node->cursor += count;
        return count;
    }

    return -1;
}

uint32_t write_lock = 0;

int devfs_write(VfsNode *node, size_t count, void *buffer)
{
    if (str_eq(node->name, "tty"))
    {

        spin_lock(&write_lock);

        char *buf = (char *)buffer;

        for (size_t i = 0; i < count; i++)
        {
            char c = buf[i];

            com_putc(c);
        }

        spin_release(&write_lock);

        node->cursor += count;
        node->stat.st_size += count;

        return count;
    }
    return -1;
}

Filesystem devfs()
{
    return (Filesystem){.read = devfs_read, .name = "DEVFS", .write = devfs_write};
}

void devfs_init()
{
    vfs_mount("/dev", devfs());
    vfs_open(vfs_get_root(), "/dev/tty", O_CREAT);
}