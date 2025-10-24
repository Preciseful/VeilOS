#include <interface/module.h>
#include <fs/voidelle.h>
#include <lib/string.h>
#include <lib/list.h>

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

        AddToList(&modules, module);

        voidelle_pos = voidelle.next;
    }
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

        if (strcmp("Modules", name) == 0)
        {
            free(name);
            set_modules(voidom, voidelle.content);
            break;
        }

        content = voidelle.next;
        free(name);
    }
}