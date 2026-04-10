/**
 * @author Developful
 * @brief VFS Voidelle filesystem file size functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>

long VoidelleIFileSize(const char *path, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle voidelle;
    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NO_FILE;

    return voidelle.content_voidelle_size;
}