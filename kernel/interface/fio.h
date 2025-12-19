#pragma once

typedef struct FilesystemInterface
{
    int (*fopen)(const char *path);
    int (*fread)(const char *path, char *buf, long offset, void *key);
} FilesystemInterface;

int OpenFile(const char *path);