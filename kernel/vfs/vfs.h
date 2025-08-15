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

typedef struct virtual_fs
{
    unsigned long roots_count, roots_capacity;
    vfs_root_t *roots;
} virtual_fs_t;

void vfs_init();
vfs_root_t *add_root(void *fs, unsigned int fs_type, char id);
virtual_fs_t *get_vfs();