#pragma once

#include <vfs/vfs.h>

enum VNode_Types
{
    VNODE_DIRECTORY,
    VNODE_FILE,
    VNODE_LINK
};

enum FSeek_Types
{
    SEEK_SET,
    SEEK_CUR,
};

typedef struct vnode
{
    char *path;
    vfs_root_t *root;
    enum VNode_Types type;
    unsigned long seek;
    void *data;
} vnode_t;

vnode_t *fopen(char *path);
void fseek(vnode_t *node, unsigned long seek, enum FSeek_Types type);
unsigned long fread(void *buf, unsigned long size, vnode_t *node);
void fwrite(void *buf, unsigned long size, vnode_t *node);