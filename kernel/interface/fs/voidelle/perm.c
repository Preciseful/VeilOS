/**
 * @author Developful
 * @brief VFS Voidelle filesystem permission functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>
#include <stdint.h>

#define USER_BITS(permission)                                          \
    (((permission & VOIDELLE_PERMISSION_EXECUTE) ? USER_EXECUTE : 0) | \
     ((permission & VOIDELLE_PERMISSION_WRITE) ? USER_WRITE : 0) |     \
     ((permission & VOIDELLE_PERMISSION_READ) ? USER_READ : 0))

#define OTHERS_BITS(permission)                                         \
    (((permission & VOIDELLE_PERMISSION_EXECUTE) ? OTHER_EXECUTE : 0) | \
     ((permission & VOIDELLE_PERMISSION_WRITE) ? OTHER_WRITE : 0) |     \
     ((permission & VOIDELLE_PERMISSION_READ) ? OTHER_READ : 0))

int VoidelleIPermissions(const char *path, void *key)
{
    Voidom voidom = *(Voidom *)key;

    Voidelle voidelle;
    if (!FindVoidelleByPath(voidom, path, &voidelle))
        return -E_NO_FILE;

    int perms = OTHERS_BITS(voidelle.other_permission) | USER_BITS(voidelle.owner_permission);
    return perms;
}