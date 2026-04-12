/**
 * @author Developful
 * @brief VFS FAT32 filesystem file size functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

long Fat32IFileSize(const char *path, void *key)
{
    FatFS *fatfs = (FatFS *)key;

    FatFSNode node;
    if (!FindFat32ByPath(fatfs, path, &node))
        return -E_NO_FILE;

    return node.entry.size;
}