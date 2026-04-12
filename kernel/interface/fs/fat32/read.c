/**
 * @author Developful
 * @brief VFS FAT32 filesystem reading functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

long Fat32IRead(const char *path, char *buf, unsigned long size, unsigned long offset, void *file, void *key)
{
    FatFSNode node;
    FatFS *fs = (FatFS *)key;

    if (!FindFat32ByPath(fs, path, &node))
        return -E_NO_FILE;

    return ReadFatNode(node, offset, size, buf);
}