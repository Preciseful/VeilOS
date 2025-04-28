#include <modules.h>
#include <lib/elf.hpp>
#include <lib/fork.h>

using namespace veil;

void init_module(const char *dir)
{
    char full[100];
    tfp_sprintf(full, "/modules/%s.elf", dir);

    File *file = File::Open(full);
    if (!file)
    {
        printf("No file exists for module (\"%s\").\n\n", dir);
        return;
    }

    ELF *bin = new ELF(file);
    if (!bin->Initialize())
    {
        printf("Module \"%s\" failed to initialize.\n\n", dir);
        return;
    }
    else
        printf("Module \"%s\" was initialized.\n\n", dir);

    fork((unsigned long)bin->Entry, 0, 1);
}

void modules_init()
{
    printf("Initializing modules..\n");
    init_module("Vela");
    init_module("Luna");
}