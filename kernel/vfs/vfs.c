#include <vfs/vfs.h>
#include <memory/memory.h>
#include <fs/fat32.h>
#include <lib/printf.h>
#include <vfs/vnode.h>

static VirtualFS *vfs = 0;

VirtualFS *get_vfs()
{
    return vfs;
}

void vfs_init()
{
    vfs = malloc(sizeof(VirtualFS));
    vfs->roots_count = 0;
    vfs->roots_capacity = 1;
    vfs->roots = malloc(sizeof(VfsRoot));
}

VfsRoot *add_root(void *fs, unsigned int fs_type, char id)
{
    if (vfs->roots_count == vfs->roots_capacity)
    {
        vfs->roots_capacity *= 2;

        VfsRoot *new_roots = malloc(vfs->roots_capacity * sizeof(VfsRoot));
        memcpy(new_roots, vfs->roots, vfs->roots_count);

        free(vfs->roots);
        vfs->roots = new_roots;
    }

    VfsRoot *current = &vfs->roots[vfs->roots_count];
    current->fs = fs;
    current->fs_type = fs_type;
    current->id = id;
    current->open_nodes = malloc(sizeof(VNode));
    current->nodes_count = 0;
    current->nodes_capacity = 1;
    vfs->roots_count++;

    return &vfs->roots[vfs->roots_count];
}