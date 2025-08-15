#include <vfs/vfs.h>
#include <memory/memory.h>
#include <fs/fat32.h>
#include <lib/printf.h>
#include <vfs/vnode.h>

static virtual_fs_t *vfs = 0;

virtual_fs_t *get_vfs()
{
    return vfs;
}

void vfs_init()
{
    vfs = malloc(sizeof(virtual_fs_t));
    vfs->roots_count = 0;
    vfs->roots_capacity = 1;
    vfs->roots = malloc(sizeof(vfs_root_t));
}

vfs_root_t *add_root(void *fs, unsigned int fs_type, char id)
{
    if (vfs->roots_count == vfs->roots_capacity)
    {
        vfs->roots_capacity *= 2;

        vfs_root_t *new_roots = malloc(vfs->roots_capacity * sizeof(vfs_root_t));
        memcpy(new_roots, vfs->roots, vfs->roots_count);

        free(vfs->roots);
        vfs->roots = new_roots;
    }

    vfs_root_t *current = &vfs->roots[vfs->roots_count];
    current->fs = fs;
    current->fs_type = fs_type;
    current->id = id;
    current->open_nodes = malloc(sizeof(vnode_t));
    current->nodes_count = 0;
    current->nodes_capacity = 1;
    vfs->roots_count++;

    return &vfs->roots[vfs->roots_count];
}