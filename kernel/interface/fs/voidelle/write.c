#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>
#include <lib/string.h>

int VoidelleIWrite(const char *path, enum File_Mode mode, const char *buf, unsigned long size, unsigned long offset, void *key)
{
    Voidelle voidelle;
    Voidom voidom = *(Voidom *)key;

    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NO_FILE;

    voidelle.content_voidelle_size = offset + size;
    write_void(voidom, &voidelle, voidelle.position, sizeof(Voidelle));

    uint64_t first_voidite = offset / VOIDITE_CONTENT_SIZE;
    uint64_t last_voidite = (offset + size - 1) / VOIDITE_CONTENT_SIZE;
    uint64_t start = offset % VOIDITE_CONTENT_SIZE;
    uint64_t end = (offset + size) % VOIDITE_CONTENT_SIZE;

    const char *current_buf = buf;

    for (unsigned long voidite_index = first_voidite; voidite_index <= last_voidite; voidite_index++)
    {
        Voidite voidite;

        if (!get_content_voidite_at(voidom, voidelle, &voidite, voidite_index))
        {
            fill_content_voidites(voidom, &voidelle, voidite_index + 1);
            get_content_voidite_at(voidom, voidelle, &voidite, voidite_index);
        }

        uint8_t *cpy_start = voidite.data;
        uint64_t cpy_len = VOIDITE_CONTENT_SIZE;

        if (voidite_index == first_voidite)
        {
            cpy_start += start;
            cpy_len -= start;
        }

        if (voidite_index == last_voidite)
            cpy_len = (end == 0) ? size : end;

        if (cpy_len > size)
            cpy_len = size;

        memcpy(cpy_start, current_buf, cpy_len);

        current_buf += cpy_len;
        size -= cpy_len;

        write_void(voidom, &voidite, voidite.position, sizeof(Voidite));
    }

    return current_buf - buf;
}