#include "lib/vec.h"
#include <fs/vfs.h>
#include <lib/alloc.h>
#include <lib/print.h>
#include <lib/str.h>

Mountpoint mountpoints[64];
size_t mountpoints_n = 0;
VfsNode *root = NULL;

VfsNode *vfs_mkdir(char *path)
{
    VfsNode *node = malloc(sizeof(VfsNode));

    node->type = VFS_DIRECTORY;

    vec_init(&node->children);

    if (!str_eq(path, "/"))
    {
        node->parent = vfs_get_parent(path);
        node->name = basename(path);

        if (!node->parent)
        {
            set_errno(ENOENT);
            return NULL;
        }

        node->mountpoint = node->parent->mountpoint;

        VfsNode *prev_dir = malloc(sizeof(VfsNode));

        prev_dir->type = VFS_DIRECTORY;

        prev_dir->name = "..";
        prev_dir->points_to = node->parent;

        VfsNode *curr_dir = malloc(sizeof(VfsNode));

        curr_dir->type = VFS_DIRECTORY;
        curr_dir->name = ".";
        curr_dir->points_to = node;

        vec_push(&node->children, prev_dir);
        vec_push(&node->children, curr_dir);
        vec_push(&node->parent->children, node);
    }

    else
    {
        node->parent = node;
        node->mountpoint = malloc(sizeof(Mountpoint));
        node->name = "/";
    }

    if (nerd_logs())
    {
        log("added directory %s, parent is %s", node->name, node->parent == root ? "/" : node->parent->name);
    }

    return node;
}

int vfs_mount(char *where, Filesystem fs)
{
    if (!root)
    {
        VfsNode *tmp = vfs_mkdir(where);

        root = tmp;

        root->mountpoint->fs = fs;

        root->mountpoint->root = root;

        mountpoints[mountpoints_n++] = *root->mountpoint;

        return 0;
    }

    VfsNode *node = vfs_mkdir(where);

    node->mountpoint->fs = fs;

    mountpoints[mountpoints_n++] = *node->mountpoint;

    return 0;
}

vec_str_t vfs_parse_path(char *path)
{
    vec_str_t segments;

    vec_init(&segments);

    char *segment = strtok(strdup(path), "/");

    while (segment)
    {
        vec_push(&segments, strdup(segment));

        segment = strtok(NULL, "/");
    }

    return segments;
}

VfsNode *vfs_find_node(VfsNode *start, char *name)
{
    if (start->points_to)
    {
        for (int i = 0; i < start->points_to->children.length; i++)
        {
            if (str_eq(name, start->points_to->children.data[i]->name))
            {
                return start->points_to->children.data[i];
            }
        }
    }

    for (int i = 0; i < start->children.length; i++)
    {
        if (str_eq(name, start->children.data[i]->name))
        {
            return start->children.data[i];
        }
    }

    return NULL;
}

VfsNode *vfs_get_parent(char *path)
{
    VfsNode *node = root;

    vec_str_t segments = vfs_parse_path(path);

    for (int i = 0; i < segments.length - 1; i++)
    {
        if (strlen(segments.data[i]) != 0)
        {

            if (i + 2 == segments.length)
            {
                return node;
            }

            node = vfs_find_node(node, segments.data[i + 1]);
        }

        else
        {
            if (segments.length == 2)
            {
                return root;
            }

            node = vfs_find_node(root, segments.data[i + 1]);
        }
    }

    if (!node)
        set_errno(ENOENT);

    return node;
}

VfsNode *vfs_get_root()
{
    return root;
}

VfsNode *vfs_open(char *path, int flags)
{
    VfsNode *parent = vfs_get_parent(path);

    if (!parent)
    {
        set_errno(ENOENT);
        return NULL;
    }

    VfsNode *node = vfs_find_node(parent, basename(path));

    if (!(flags & O_DIRECTORY) && node)
    {
        if (node->type == VFS_DIRECTORY)
        {
            set_errno(EISDIR);
            return NULL;
        }
    }

    if (flags & O_CREAT)
    {
        if (node)
        {
            node->o_mode = flags;

            return node;
        }

        VfsNode *ret = malloc(sizeof(VfsNode));

        ret->name = strdup(basename(path));

        ret->type = VFS_FILE;

        ret->parent = parent;

        ret->o_mode = flags;

        ret->mountpoint = ret->parent->mountpoint;

        ret->address = (uintptr_t)malloc(2);

        vec_push(&ret->parent->children, ret);

        return ret;
    }

    if (!node)
    {
        set_errno(ENOENT);
        return NULL;
    }

    node->o_mode = flags;

    if (node->points_to)
        return node->points_to;

    return node;
}

int vfs_read(VfsNode *vfs_node, size_t offset, size_t count, void *buf)
{
    return vfs_node->mountpoint->fs.read(vfs_node, offset, count, buf);
}

int vfs_write(VfsNode *vfs_node, size_t count, void *buf)
{
    return vfs_node->mountpoint->fs.write(vfs_node, count, buf);
}
