/**
 * @file
 * @author Developful
 * @brief General interface of all filesystems for the VFS.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <scheduler/task.h>

typedef long FILEHANDLE;

enum File_Mode
{
    FILE_WRITE = 1 << 0,
    FILE_READ = 1 << 1,
    FILE_EXECUTE = 1 << 2,
};

enum File_Permissions
{
    USER_EXECUTE = 1 << 0,
    USER_READ = 1 << 1,
    USER_WRITE = 1 << 2,
    OTHER_EXECUTE = 1 << 3,
    OTHER_READ = 1 << 4,
    OTHER_WRITE = 1 << 5
};

typedef unsigned char FILEMODE;

typedef struct FilesystemInterface
{
    int (*fcreate_file)(const char *path, enum File_Permissions permissions, void *key);
    int (*fcreate_directory)(const char *path, enum File_Permissions permissions, void *key);
    int (*fopen)(const char *path, enum File_Mode mode, void **file, void *key);
    long (*fread)(const char *path, char *buf, unsigned long size, unsigned long offset, void *file, void *key);
    long (*fwrite)(const char *path, const char *buf, unsigned long size, unsigned long offset, void *file, void *key);
    long (*fsize)(const char *path, void *key);
    int (*fpermissions)(const char *path, void *key);

    void *key;
} FilesystemInterface;

const char *GetFilename(const char *path);

/**
 * @brief Create a file at a path.
 *
 * @param path The path.
 * @param permissions The file's permissions.
 * @return Possible errors.
 */
int CreateFile(const char *path, enum File_Permissions permissions);

/**
 * @brief Create a directory at a path.
 *
 * @param path The path.
 * @param permissions The directory's permissions.
 * @return Possible errors.
 */
int CreateDirectory(const char *path, enum File_Permissions permissions);

/**
 * @brief Opens a file found at the path in a certain mode.
 *
 * @param mode The mode to open the file in.
 * @param path The path for its location.
 * @return The file handle.
 */
FILEHANDLE OpenFile(enum File_Mode mode, const char *path);

/**
 * @brief Closes a file.
 *
 * @param handle The file handle.
 */
void CloseFile(FILEHANDLE handle);

/**
 * @brief Reads a certain amount of contents from the file into a buffer, starting at an offset.
 *
 * @param handle The file handle.
 * @param buf The buffer.
 * @param size The size of the buffer.
 * @param offset The offset in file from which the read starts from.
 * @return The amount read, or possible errors.
 */
int ReadFile(FILEHANDLE handle, void *buf, unsigned long size, unsigned long offset);

/**
 * @brief Writes to a file at a certain offset from a buffer.
 *
 * @param handle The file handle.
 * @param buf The buffer.
 * @param size The size of the buffer.
 * @param offset The offset in file from which the write starts from.
 * @return The amount written, or possible errors.
 */
int WriteFile(FILEHANDLE handle, const char *buf, unsigned long size, unsigned long offset);

/**
 * @brief Get a file's size.
 *
 * @param path The path of the file.
 * @return The size.
 */
long GetFileSize(const char *path);