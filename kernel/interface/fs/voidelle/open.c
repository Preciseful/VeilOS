/**
 * @author Developful
 * @brief VFS Voidelle filesystem opening functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>
#include <stdint.h>

int VoidelleIOpen(const char *path, enum File_Mode mode, void **file, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle *voidelle = malloc(sizeof(Voidelle));
    if (!FindVoidelleByPath(voidom, path, voidelle))
        return -E_NO_FILE;

    *file = voidelle;
    return 0;
}