#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H
#include <lib/base.h>
#include <lib/vec.h>

struct mountpoint;

typedef enum
{
    VFS_FILE,
    VFS_DIRECTORY,
} VfsNodeType;

typedef struct vfs_node
{
    VfsNodeType type;

    char *name;

    struct vfs_node *parent;
    struct vfs_node *points_to;

    vec_t(struct vfs_node *) children;

    struct mountpoint *mountpoint;

    struct stat stat;

    size_t cursor;
    int o_mode;

    uintptr_t address;

} VfsNode;

typedef struct
{
    int (*read)(VfsNode *vfs_node, size_t offset, size_t count, void *buf);
    int (*write)(VfsNode *vfs_node, size_t count, void *buf);
    uintptr_t (*open)(char *path);
    char *name;
} Filesystem;

typedef struct mountpoint
{
    Filesystem fs;
    VfsNode *root;
} Mountpoint;

int vfs_mount(char *where, Filesystem fs);

VfsNode *vfs_open(char *path, int flags);

int vfs_read(VfsNode *vfs_node, size_t offset, size_t count, void *buf);

VfsNode *vfs_mkdir(char *path);

VfsNode *vfs_get_root();

VfsNode *vfs_get_parent(char *path);

int vfs_write(VfsNode *vfs_node, size_t count, void *buf);

#endif
