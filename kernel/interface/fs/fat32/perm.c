/**
 * @author Developful
 * @brief VFS FAT32 filesystem permission functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

// todo : maybe get permissions from a file ?
int Fat32IDirectPermissions()
{
    return (USER_EXECUTE | USER_READ | USER_WRITE) |
           (OTHER_EXECUTE | OTHER_READ | OTHER_WRITE);
}

int Fat32IPermissions(const char *path, void *key)
{
    FatFS *fatfs = (FatFS *)key;
    if (!FindFat32ByPath(fatfs, path, 0))
        return -E_NO_FILE;

    return Fat32IDirectPermissions();
}