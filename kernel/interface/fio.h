#pragma once

typedef struct FilesystemInterface
{
    int (*fopen)(const char *path);
    int (*fread)(const char *path, char *buf, unsigned long size, unsigned long offset, void *key);
    int (*fwrite)(const char *path, const char *buf, unsigned long size, unsigned long offset, void *key);
} FilesystemInterface;

int OpenFile(const char *path);