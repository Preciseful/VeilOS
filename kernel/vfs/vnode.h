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

typedef struct VNode
{
    const char *path;
    VfsRoot *root;
    enum VNode_Types type;
    unsigned long seek;
    void *data;
} VNode;

VNode *OpenFile(const char *path);
void CloseFile(VNode *node);
void SeekInFile(VNode *node, unsigned long seek, enum FSeek_Types type);
unsigned long ReadFile(void *buf, unsigned long size, VNode *node);
void WriteToFile(void *buf, unsigned long size, VNode *node);