#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>
#include <lib/string.h>
#include <memory/memory.h>

int VoidelleIReadDir(const char *path, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle voidelle;
    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NO_FILE;

    uint64_t pos = voidelle.content_voidelle;

    while (pos)
    {
        read_void(voidom, &voidelle, pos, sizeof(Voidelle));

        char name[voidelle.name_voidelle_size];
        get_voidelle_name(voidom, voidelle, name);

        pos = voidelle.next_voidelle;
    }

    return 0;
}