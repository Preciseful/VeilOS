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
#include <lib/printf.h>

#define FLAGS(v)                                    \
    (((v & VOIDELLE_DIRECTORY) ? FFDIRECTORY : 0) | \
     ((v & VOIDELLE_HIDDEN) ? FFHIDDEN : 0) |       \
     ((v & VOIDELLE_SYSTEM) ? FFSYSTEM : 0))

int VoidelleIOpen(const char *path, enum File_Mode mode, FileMeta *meta, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle *voidelle = malloc(sizeof(Voidelle));

    if (!FindVoidelleByPath(voidom, path, voidelle))
        return -E_NO_FILE;

    meta->file_data = voidelle;
    meta->flags = FLAGS(voidelle->flags);
    meta->permissions = VoidelleIDirectPermissions(*voidelle);
    meta->owner_id = voidelle->owner_id;
    return 0;
}