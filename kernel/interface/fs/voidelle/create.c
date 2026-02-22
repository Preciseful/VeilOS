#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <interface/errno.h>
#include <stdint.h>

#define USER_BITS(permission)                                          \
    (((permission & USER_EXECUTE) ? VOIDELLE_PERMISSION_EXECUTE : 0) | \
     ((permission & USER_WRITE) ? VOIDELLE_PERMISSION_WRITE : 0) |     \
     ((permission & USER_READ) ? VOIDELLE_PERMISSION_READ : 0))

#define OTHER_BITS(permission)                                          \
    (((permission & OTHER_EXECUTE) ? VOIDELLE_PERMISSION_EXECUTE : 0) | \
     ((permission & OTHER_WRITE) ? VOIDELLE_PERMISSION_WRITE : 0) |     \
     ((permission & OTHER_READ) ? VOIDELLE_PERMISSION_READ : 0))

int VoidelleICreate(const char *path, enum File_Permissions permissions, void *key, bool dir)
{
    Voidom voidom = *(Voidom *)key;

    if (FindVoidelleByPath(voidom, path, 0))
        return -E_NO_FILE;

    enum Voidelle_Flags dir_flag = dir ? VOIDELLE_DIRECTORY : 0;
    uint64_t uid = 0;
    uint8_t owner_perm = USER_BITS(permissions);
    uint8_t other_perm = OTHER_BITS(permissions);

    Voidelle voidelle;
    create_voidelle(voidom, &voidelle, GetFilename(path), dir_flag, uid, owner_perm, other_perm);

    Voidelle parent;
    FindParentVoidelleByPath(voidom, path, &parent);

    add_voidelle(voidom, &parent, &voidelle);

    return 0;
}

int VoidelleICreateDirectory(const char *path, enum File_Permissions permissions, void *key)
{
    return VoidelleICreate(path, permissions, key, true);
}

int VoidelleICreateFile(const char *path, enum File_Permissions permissions, void *key)
{
    return VoidelleICreate(path, permissions, key, false);
}