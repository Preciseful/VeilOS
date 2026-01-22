#pragma once

#include <stdbool.h>
#include <interface/fio.h>
#include <scheduler/task.h>

// a hard coded mount count is enough for now
#define MOUNT_COUNT 128

typedef struct MountPoint
{
    char *path;
    unsigned int hash_value;
    FilesystemInterface fs;
    void *key;
} MountPoint;

typedef struct FileReference
{
    PID owner;
    FILEMODE mode;
    const char *path;
} FileReference;

typedef struct VFS
{
    MountPoint mounts[MOUNT_COUNT];
    FileReference **files;
    unsigned long files_count;
} VFS;

void VFSInit();
void AddMountPoint(const char *path, FilesystemInterface interface);
bool GetMountPoint(const char *path, MountPoint *point_buf, char **extra_path);

FILEHANDLE AddFileReference(FileReference reference);
void RemoveFileReference(FILEHANDLE handle);
