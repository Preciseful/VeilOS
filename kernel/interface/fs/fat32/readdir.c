/**
 * @author Developful
 * @brief VFS FAT32 filesystem reading directories functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

int Fat32IReadDir(const char *path, void *key)
{
    FatFS *fatfs = (FatFS *)key;
    FatFSNode node;

    if (!FindFat32ByPath(fatfs, path, &node))
        return -E_NO_FILE;

    FatFSNode *nodes;
    unsigned long count = GetFatEntries(fatfs, node.cluster, &nodes);

    for (unsigned long i = 0; i < count; i++)
    {
        // todo : fill
    }

    return 0;
}