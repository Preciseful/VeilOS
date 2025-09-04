#pragma once

#include <lib/list.h>
#include <interface/portal.h>
#include <stdbool.h>

#define VFS_MAX_ROOTS 32
#define FS_GET_ROOT_ID 0
#define FS_REQUEST_FILE 1

typedef unsigned long FileRequestID;
typedef int FileID;

typedef struct FSObject
{
    unsigned long seek;
    void *fs;
    FileRequestID id;
} FSObject;

typedef struct VEntry
{
    const char *path;
    // index in path where the name starts
    unsigned long name_position;
    Portal portal;

    List cached_entries;
    FileRequestID request_id;
    // distinct entries are used to maintain entries with a distinct filesystem
    // example: /dev on linux, may be held here in order to not be forgotten
    List distinct_entries;
} VEntry;

typedef struct VFS
{
    VEntry roots[VFS_MAX_ROOTS];
} VFS;

void VFSInit();
bool AddRoot(const char *path, Portal filesystem);
FileID OpenFile(const char *path);