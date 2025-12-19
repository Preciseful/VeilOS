#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>

int fread(const char *path, char *buf, unsigned long size, unsigned long offset, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle voidelle;
    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NOFILE;

    return read_voidelle(voidom, voidelle, offset, buf, size);
}
