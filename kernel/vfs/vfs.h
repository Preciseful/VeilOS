#pragma once

enum Filesystems
{
    FAT32,
    VOIDELLE
};

typedef struct VNode VNode;
typedef struct VfsRoot
{
    char id;
    enum Filesystems fs_type;
    void *fs;
    unsigned long nodes_count, nodes_capacity;
    VNode **open_nodes;
} VfsRoot;

typedef struct VirtualFS
{
    unsigned long roots_count, roots_capacity;
    VfsRoot *roots;
} VirtualFS;

void VfsInit();
VfsRoot *AddRootToVfs(void *fs, unsigned int fs_type, char id);
VirtualFS *get_vfs();