#pragma once

#include <vfs/vfs.h>

enum VNode_Types
{
    VNODE_DIRECTORY,
    VNODE_FILE,
    VNODE_LINK
};

typedef struct vnode
{
    char *path;
    enum FileSystems fs_type;
    enum VNode_Types type;
    void *data;
} vnode_t;

vnode_t *fopen(char *path);