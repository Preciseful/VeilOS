#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>
#include <lib/string.h>

int VoidelleIWrite(const char *path, const char *buf, unsigned long size, unsigned long offset, void *key)
{
    Voidelle voidelle;
    Voidom voidom = *(Voidom *)key;

    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NOFILE;

    if (!voidelle.content_voidelle)
    {
        Voidite content;
        content.position = get_free_void(voidom);
        content.next_voidite = 0;
        memset(content.data, 0, VOIDITE_CONTENT_SIZE);

        voidelle.content_voidelle = content.position;

        write_void(voidom, &voidelle, voidelle.position, sizeof(Voidelle));
        write_void(voidom, &content, content.position, sizeof(Voidite));
    }

    unsigned long new_size = offset + size;

    if (new_size > voidelle.content_voidelle_size)
    {
        unsigned long voidites_count = (new_size + (VOIDITE_CONTENT_SIZE - 1)) / VOIDITE_CONTENT_SIZE;
        Voidite last_voidite;

        voidelle.content_voidelle_size = new_size;
        write_void(voidom, &voidelle, voidelle.position, sizeof(Voidelle));

        unsigned long pos = voidelle.content_voidelle;
        while (pos)
        {
            read_void(voidom, &last_voidite, pos, sizeof(Voidite));
            pos = last_voidite.next_voidite;
            voidites_count--;
        }

        for (unsigned long i = 0; i < voidites_count; i++)
        {
            Voidite next_voidite;
            next_voidite.next_voidite = 0;
            next_voidite.position = get_free_void(voidom);
            memset(next_voidite.data, 0, VOIDITE_CONTENT_SIZE);

            last_voidite.next_voidite = next_voidite.position;

            write_void(voidom, &last_voidite, last_voidite.position, sizeof(Voidite));
            write_void(voidom, &next_voidite, next_voidite.position, sizeof(Voidite));

            last_voidite = next_voidite;
        }
    }

    unsigned long voidite_start = offset / VOIDITE_CONTENT_SIZE;
    unsigned long voidite_offset = offset % VOIDITE_CONTENT_SIZE;
    unsigned long voidite_pos = voidite_start;

    int written = 0;
    while (size)
    {
        Voidite voidite;
        get_content_voidite_at(voidom, voidelle, &voidite, voidite_pos);

        unsigned long bytes_count;

        if (voidite_pos == voidite_start)
            bytes_count = VOIDITE_CONTENT_SIZE - voidite_offset;
        else
            bytes_count = size;

        if (bytes_count > size)
            bytes_count = size;

        memcpy(voidite.data + (voidite_pos == voidite_start ? voidite_offset : 0),
               buf + written, bytes_count);

        write_void(voidom, &voidite, voidite.position, sizeof(Voidite));

        written += bytes_count;
        size -= bytes_count;

        voidite_pos = voidite.next_voidite;
    }

    return written;
}