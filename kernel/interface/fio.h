#pragma once

#include <scheduler/task.h>

typedef long FILEHANDLE;

enum File_Mode
{
    FILE_CREATE = 1 << 0,
    FILE_WRITE = 1 << 1,
    FILE_READ = 1 << 2,
    FILE_EXECUTE = 1 << 3,
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
    int (*fopen)(const char *path, enum File_Mode mode, void *key);
    int (*fread)(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key);
    int (*fwrite)(const char *path, enum File_Mode mode, const char *buf, unsigned long size, unsigned long offset, void *key);
} FilesystemInterface;

int CreateFile(const char *path, enum File_Permissions permissions);
int CreateDirectory(const char *path, enum File_Permissions permissions);
FILEHANDLE OpenFile(enum File_Mode mode, const char *path);
void CloseFile(FILEHANDLE handle);
int ReadFile(FILEHANDLE handle, void *buf, unsigned long size, unsigned long offset);
int WriteFile(FILEHANDLE handle, const char *buf, unsigned long size, unsigned long offset);