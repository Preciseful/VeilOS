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
#include <lib/printf.h>

#define FLAGS(v)                      \
    (((v & 0x10) ? FFDIRECTORY : 0) | \
     ((v & 0x02) ? FFHIDDEN : 0) |    \
     ((v & 0x04) ? FFSYSTEM : 0))

int Fat32IOpen(const char *path, enum File_Mode mode, FileMeta *meta, void *key)
{
    FatFSNode *node = malloc(sizeof(FatFSNode));
    FatFS *fs = (FatFS *)key;

    if (!FindFat32ByPath(fs, path, node))
        return -E_NO_FILE;

    meta->file_data = node;
    meta->flags = FLAGS(node->entry.attrs);
    meta->permissions = Fat32IDirectPermissions();
    meta->owner_id = 1;

    return 0;
}