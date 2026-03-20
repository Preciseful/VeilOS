#pragma once

#include <stdbool.h>
#include <fs/voidelle.h>
#include <interface/fio.h>
#include <fs/fat32.h>

int Fat32IRead(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key);

FilesystemInterface GetFat32Interface(FatFS *fatfs);