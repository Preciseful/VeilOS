#pragma once

#include <fs/voidelle.h>
#include <lib/list.h>

typedef struct Module
{
    char *name;
    Voidom voidom;
    Voidelle voidelle;
} Module;

List GetModules();
void ModulesInit(Voidom voidom);
void StartModule(const char *name);