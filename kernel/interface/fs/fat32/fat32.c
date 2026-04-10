/**
 * @author Developful
 * @brief VFS interface for the FAT32 filesystem.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

FilesystemInterface GetFat32Interface(FatFS *fatfs)
{
    FilesystemInterface interface;
    interface.fread = Fat32IRead;
    interface.fopen = 0;
    interface.fwrite = 0;
    interface.fcreate_directory = 0;
    interface.fcreate_file = 0;
    interface.fsize = 0;
    interface.key = fatfs;

    return interface;
}