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

bool FindParentFat32ByPath(FatFS *fs, const char *path, FatFSNode *buf);
bool FindFat32ByPath(FatFS *fs, const char *path, FatFSNode *buf);

long Fat32IRead(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key);
long Fat32IWrite(const char *path, enum File_Mode mode, const char *buf, unsigned long size, unsigned long offset, void *key);
int Fat32ICreateDirectory(const char *path, enum File_Permissions permissions, void *key);
int Fat32ICreateFile(const char *path, enum File_Permissions permissions, void *key);
long Fat32IFileSize(const char *path, void *key);

FilesystemInterface GetFat32Interface(FatFS *fatfs);