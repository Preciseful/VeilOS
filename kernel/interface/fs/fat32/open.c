/**
 * @author Developful
 * @brief VFS FAT32 filesystem opening functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

int Fat32IOpen(const char *path, enum File_Mode mode, void **file, void *key)
{
    FatFSNode *node = malloc(sizeof(FatFSNode));
    FatFS *fs = (FatFS *)key;

    if (!FindFat32ByPath(fs, path, node))
        return -E_NO_FILE;

    *file = node;
    return 0;
}