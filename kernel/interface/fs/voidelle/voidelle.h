#pragma once

#include <stdbool.h>
#include <fs/voidelle.h>
#include <interface/fio.h>

const char *GetFilename(const char *path);
bool FindParentVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf);
bool FindVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf);

int VoidelleIRead(const char *path, char *buf, unsigned long size, unsigned long offset, void *key);
int VoidelleIWrite(const char *path, const char *buf, unsigned long size, unsigned long offset, void *key);

FilesystemInterface GetVoidelleInterface();