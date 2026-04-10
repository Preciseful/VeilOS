/**
 * @author Developful
 * @brief VFS Voidelle filesystem reading functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>

long VoidelleIRead(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle voidelle;
    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NO_FILE;

    return read_voidelle(voidom, voidelle, offset, buf, size);
}
