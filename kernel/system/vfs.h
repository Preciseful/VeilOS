#pragma once

#include <stdbool.h>
#include <interface/fio.h>

// a hard coded mount point is enough for now
#define MOUNT_COUNT 128

typedef struct MountPoint
{
    char *path;
    unsigned int hash_value;
    FilesystemInterface fs;
} MountPoint;

typedef struct VFS
{
    MountPoint mounts[MOUNT_COUNT];
} VFS;

void VFSInit();
void AddMountPoint(const char *path, FilesystemInterface interface);
bool GetMountPoint(const char *path, MountPoint *point_buf, char **extra_path);