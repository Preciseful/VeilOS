/**
 * @file
 * @author Developful
 * @brief VFS interface.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    enum File_Mode mode;
    const char *path;
    unsigned long mount_idx;
    char *cut_path;
    bool used;
    void *file_data;
} FileReference;

typedef struct VFS
{
    MountPoint mounts[MOUNT_COUNT];
    FileReference *files;
    unsigned long files_count;
} VFS;

/**
 * @brief Initializes the VFS.
 */
void VFSInit();

/**
 * @brief Adds a mounting point for a filesystem
 *
 * @param path The path for the mounting point.
 * @param interface The filesystem's interface.
 */
void AddMountPoint(const char *path, FilesystemInterface interface);

/**
 * @brief Get the mounting point of a filesystem from the path.
 *
 * @param path The path.
 * @param[out] point_buf The mounting point.
 * @param[out] extra_path The rest of the path starting from the mounting point, similar to an absolute path.
 * @return `true` if found, otherwise `false`.
 */
bool GetMountPoint(const char *path, MountPoint *point_buf, char **extra_path);

/**
 * @brief Adds a file reference in the VFS.
 *
 * @param reference The reference.
 * @return The file handle for the reference.
 */
FILEHANDLE AddFileReference(FileReference reference);

/**
 * @brief Frees up the file reference from the VFS.
 *
 * @param handle The file handle.
 */
void RemoveFileReference(FILEHANDLE handle);

/**
 * @brief Gets the file reference from a file handle.
 *
 * @param handle The file handle.
 * @param[out] reference The reference.
 * @return `true` if found, otherwise `false`.
 */
bool GetFileReference(FILEHANDLE handle, FileReference *reference);

/**
 * @brief Gets the file's mounting point from a file handle.
 *
 * @param handle The file handle
 * @param[out] mount The mounting point.
 * @return `true` if found, otherwise `false`.
 */
bool GetFileMount(FILEHANDLE handle, MountPoint *mount);
