#include <system/vfs.h>
#include <memory/memory.h>
#include <lib/string.h>
#include <limits.h>
#include <lib/printf.h>

VFS *vfs;

static inline unsigned int murmur_32_scramble(unsigned int k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;

    return k;
}

unsigned int murmurHash(const char *key, unsigned long len)
{
    unsigned int h = 0x0;
    unsigned int k;

    for (unsigned long i = len >> 2; i; i--)
    {
        memcpy(&k, key, sizeof(unsigned int));
        key += sizeof(unsigned int);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    k = 0;

    for (unsigned long i = len & 3; i; i--)
    {
        k <<= 8;
        k |= key[i - 1];
    }

    h ^= murmur_32_scramble(k);

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

void VFSInit()
{
    vfs = malloc(sizeof(VFS));
    memset(vfs, 0, sizeof(VFS));
    vfs->files = malloc(sizeof(FileReference *) * 20);
    vfs->files_count = 20;
}

void AddMountPoint(const char *path, FilesystemInterface interface)
{
    unsigned long path_len = strlen(path);
    unsigned int hash = murmurHash(path, path_len);
    MountPoint *mp = &vfs->mounts[hash & (MOUNT_COUNT - 1)];

    mp->hash_value = hash;
    mp->path = malloc(path_len);
    mp->fs = interface;
    mp->key = 0;
    memcpy(mp->path, path, path_len);
}

bool GetMountPoint(const char *path, MountPoint *point_buf, char **extra_path)
{
    unsigned long path_len = strlen(path);

    if (path[0] != '/')
        return false;

    bool found = false;
    unsigned long best_i = 0;
    MountPoint best_mp;

    for (unsigned long i = 1; i <= path_len; i++)
    {
        if (path[i] != '/' && path[i] != '\0')
            continue;

        unsigned int hash = murmurHash(path, i);
        MountPoint mp = vfs->mounts[hash & (MOUNT_COUNT - 1)];

        if (mp.hash_value == hash)
        {
            found = true;
            best_mp = mp;
            best_i = i;
        }
    }

    if (found)
    {
        if (extra_path)
        {
            *extra_path = malloc(path_len - best_i + 1);
            memcpy(*extra_path, path + best_i, path_len - best_i + 1);
        }

        if (point_buf)
            *point_buf = best_mp;

        return true;
    }

    if (extra_path)
    {
        *extra_path = malloc(path_len - best_i + 1);
        memcpy(*extra_path, path, path_len + 1);
    }

    unsigned int hash = murmurHash("/", 1);
    MountPoint root_mp = vfs->mounts[hash & (MOUNT_COUNT - 1)];

    if (root_mp.hash_value != hash)
        return false;

    if (point_buf)
        *point_buf = root_mp;
    return true;
}

FILEHANDLE AddFileReference(FileReference reference)
{
    FILEHANDLE i;
    for (i = 0; i < vfs->files_count; i++)
    {
        if (!vfs->files[i])
            break;
    }

    if (i == vfs->files_count)
    {
        FileReference **old_files = vfs->files;
        vfs->files = malloc(sizeof(FileReference *) * vfs->files_count * 2);
        memcpy(vfs->files, old_files, sizeof(FileReference *) * vfs->files_count);

        vfs->files_count *= 2;
        free(old_files);
    }

    vfs->files[i] = malloc(sizeof(FileReference));
    memcpy(vfs->files[i], &reference, sizeof(FileReference));

    return i;
}

void RemoveFileReference(FILEHANDLE handle)
{
    if (!vfs->files[handle])
        return;

    free(vfs->files[handle]);
    vfs->files[handle] = 0;
}