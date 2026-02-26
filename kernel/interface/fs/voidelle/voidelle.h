#pragma once

#include <stdbool.h>
#include <fs/voidelle.h>
#include <interface/fio.h>

const char *GetFilename(const char *path);
bool FindParentVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf);
bool FindVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf);

int VoidelleIRead(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key);
int VoidelleIWrite(const char *path, enum File_Mode mode, const char *buf, unsigned long size, unsigned long offset, void *key);
int VoidelleICreateDirectory(const char *path, enum File_Permissions permissions, void *key);
int VoidelleICreateFile(const char *path, enum File_Permissions permissions, void *key);

FilesystemInterface GetVoidelleInterface(Voidom *voidom);