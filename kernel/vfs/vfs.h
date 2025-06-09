#pragma once

enum FileSystems
{
    FAT32,
    VOIDELLE
};

typedef struct vnode vnode_t;
typedef struct vfs_root
{
    char id;
    enum FileSystems fs_type;
    void *fs;
    unsigned long nodes_count, nodes_capacity;
    vnode_t **open_nodes;
} vfs_root_t;

// we do the later include and the early typedef to avoid circular dependency errors
// this sucks
#include <vfs/vnode.h>

typedef struct virtual_fs
{
    unsigned long roots_count, roots_capacity;
    vfs_root_t *roots;
} virtual_fs_t;

extern virtual_fs_t *vfs;

void vfs_init();
vfs_root_t *add_root(void *fs, unsigned int fs_type, char id);