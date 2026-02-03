#pragma once

#include <scheduler/task.h>

typedef long FILEHANDLE;
typedef unsigned char FILEMODE;

typedef struct FilesystemInterface
{
    int (*fopen)(const char *path);
    int (*fread)(const char *path, char *buf, unsigned long size, unsigned long offset, void *key);
    int (*fwrite)(const char *path, const char *buf, unsigned long size, unsigned long offset, void *key);
} FilesystemInterface;

FILEHANDLE OpenFile(PID pid, FILEMODE mode, const char *path);
void CloseFile(FILEHANDLE handle);
int ReadFile(FILEHANDLE handle, void *buf, unsigned long size, unsigned long offset);