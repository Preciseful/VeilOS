/**
 * @author Developful
 * @brief VFS FAT32 filesystem creating functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

int Fat32ICreate(const char *path, enum File_Permissions permissions, void *key, bool dir)
{
    FatFS *fatfs = (FatFS *)key;

    if (FindFat32ByPath(fatfs, path, 0))
        return -E_FILE_EXISTS;

    FatFSNode parent;
    FindParentFat32ByPath(fatfs, path, &parent);

    CreateFatNode(fatfs, parent.cluster, GetFilename(path), dir ? 0x10 : 0x0);
    return 0;
}

int Fat32ICreateDirectory(const char *path, enum File_Permissions permissions, void *key)
{
    return Fat32ICreate(path, permissions, key, true);
}

int Fat32ICreateFile(const char *path, enum File_Permissions permissions, void *key)
{
    return Fat32ICreate(path, permissions, key, false);
}