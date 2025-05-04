#include <modules.h>
#include <lib/elf.hpp>
#include <lib/fork.h>

using namespace veil;

void init_module(const char *dir, unsigned long flags)
{
    char full[100];
    tfp_sprintf(full, "/modules/%s.elf", dir);

    File *file = File::Open(full);
    if (!file)
    {
        ERROR("No file exists for module (\"%s\").\n\n", dir);
        return;
    }

    ELF *bin = new ELF(file);
    if (!bin->Initialize())
    {
        ERROR("Module \"%s\" failed to initialize.\n\n", dir);
        return;
    }
    else
        SUCCESS("Module \"%s\" was initialized.\n\n", dir);

    fork((unsigned long)bin->Entry, 0, flags, 0);
}

void modules_init()
{
    INFO("Initializing modules..\n");
    init_module("Vela", KERNEL_FLAG);
    init_module("Luna", SHELL_FLAG | KERNEL_FLAG);
}