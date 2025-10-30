#include <interface/module.h>
#include <fs/voidelle.h>
#include <lib/string.h>
#include <lib/list.h>
#include <memory/memory.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <lib/printf.h>
#include <elf.h>

List modules;

List GetModules()
{
    return modules;
}

void set_modules(Voidom voidom, unsigned long voidelle_pos)
{
    while (voidelle_pos)
    {
        Voidelle voidelle;
        ReadVoid(voidom, &voidelle, voidelle_pos);

        Module *module = malloc(sizeof(Module));
        module->voidelle = voidelle;
        module->voidom = voidom;
        module->name = malloc(voidelle.name_size);
        GetVoidelleName(voidom, voidelle, module->name);

        LOG("Found module: '%s'\n", module->name);

        AddToList(&modules, module);

        voidelle_pos = voidelle.next;
    }
}

Task *create_module_task(Module *module)
{
    Elf64_Ehdr eheader;
    ReadVoidelleAt(module->voidom, module->voidelle, 0, &eheader, sizeof(eheader));

    if (memcmp(eheader.e_ident, ELFMAG, 4) != 0)
    {
        LOG("ELF file provided is not of the corresponding type.\n");
        return 0;
    }

    if (eheader.e_machine != EM_AARCH64)
    {
        LOG("ELF file is not of type AARCH64.\n");
        return 0;
    }

    Task *task = CreateTask(module->name, false, eheader.e_entry, 0, 0, 0, 0);
    char user = MMU_USER;
    char exec = MMU_USER_EXEC;
    char rw = MMU_RWRW;

    for (unsigned long i = 0; i < eheader.e_phnum; i++)
    {
        Elf64_Phdr phdr;
        ReadVoidelleAt(module->voidom, module->voidelle, eheader.e_phoff + i * eheader.e_phentsize, &phdr, sizeof(phdr));

        if (phdr.p_type != PT_LOAD)
            continue;

        // align down both of these to page
        unsigned long phdr_offset = (phdr.p_offset / PAGE_SIZE) * PAGE_SIZE;
        unsigned long phdr_vaddr = (phdr.p_vaddr / PAGE_SIZE) * PAGE_SIZE;

        // pad the two
        unsigned long pad_front = phdr.p_offset - phdr_offset;
        unsigned long phdr_filesz = phdr.p_filesz + pad_front;
        unsigned long phdr_memsz = phdr.p_memsz + pad_front;

        unsigned char *read = malloc(phdr_memsz);
        memset(read, 0, phdr_memsz);

        ReadVoidelleAt(module->voidom, module->voidelle, phdr_offset, read, phdr_filesz);

        MapTaskPage(task, phdr_vaddr, VIRT_TO_PHYS(read), phdr_memsz, user | exec | rw);
    }

    task->pid = AddTask(task);
    return task;
}

void ModulesInit(Voidom voidom)
{
    unsigned long content = voidom.root.content;

    while (content)
    {
        Voidelle voidelle;
        ReadVoid(voidom, &voidelle, content);
        char *name = malloc(voidelle.name_size);
        GetVoidelleName(voidom, voidelle, name);

        if (strcmp("modules", name) == 0)
        {
            free(name);
            set_modules(voidom, voidelle.content);
            break;
        }

        content = voidelle.next;
        free(name);
    }
}

void StartModule(const char *name)
{
    LIST_FOREACH(Module, module, modules)
    {
        if (strcmp(module->name, name) != 0)
            continue;

        create_module_task(module);
        break;
    }
}