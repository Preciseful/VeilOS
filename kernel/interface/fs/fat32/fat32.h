/**
 * @author Developful
 * @brief VFS interfacing functions for the FAT32 filesystem.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <fs/voidelle.h>
#include <interface/fio.h>
#include <fs/fat32.h>

long Fat32IRead(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key);

FilesystemInterface GetFat32Interface(FatFS *fatfs);