#pragma once

typedef int VexPerm;

typedef struct __attribute__((packed)) VexPermissions
{
    VexPerm portal;
} VexPermissions;

typedef struct Vex
{
    VexPermissions permissions;
    const char *path;
} Vex;

Vex *MakeVexProcess(const char *path);