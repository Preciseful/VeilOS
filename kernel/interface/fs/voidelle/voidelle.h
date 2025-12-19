#pragma once

#include <stdbool.h>
#include <fs/voidelle.h>

const char *GetFilename(const char *path);
bool FindParentVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf);
bool FindVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf);