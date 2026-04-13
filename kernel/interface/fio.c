/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <system/vfs.h>
#include <interface/errno.h>
#include <scheduler/scheduler.h>
#include <lib/printf.h>
#include <lib/string.h>

int createEntry(const char *path, enum File_Permissions permissions, bool dir)
{
    MountPoint mount;
    char *extra;
    GetMountPoint(path, &mount, &extra);

    if (dir && mount.fs.fcreate_directory == 0)
    {
        free(extra);
        return -E_INVALID_OPERATION;
    }

    if (!dir && mount.fs.fcreate_file == 0)
    {
        free(extra);
        return -E_INVALID_OPERATION;
    }

    int ret;
    if (dir)
        ret = mount.fs.fcreate_directory(extra, permissions, mount.key);
    else
        ret = mount.fs.fcreate_file(extra, permissions, mount.key);

    free(extra);
    return ret;
}

int CreateFile(const char *path, enum File_Permissions permissions)
{
    return createEntry(path, permissions, false);
}

int CreateDirectory(const char *path, enum File_Permissions permissions)
{
    return createEntry(path, permissions, true);
}

FILEHANDLE OpenFile(enum File_Mode mode, const char *path)
{
    FileReference reference;
    reference.owner_process = GetCurrentPID();
    reference.mode = mode;
    reference.path = path;
    reference.used = true;
    reference.meta.file_data = 0;
    reference.meta.flags = 0;
    reference.meta.permissions = 0;
    reference.meta.owner_id = 1;

    MountPoint mount;
    char *extra;
    GetMountPoint(path, &mount, &extra);

    if (mount.fs.fopen)
    {
        int ret = mount.fs.fopen(extra, mode, &reference.meta, mount.key);

        if (ret != 0)
        {
            free(extra);
            return ret;
        }
    }

    free(extra);

    User owner;
    if (!GetUser(reference.meta.owner_id, &owner))
    {
        LOG("User %lu from file '%s' does not exist.\n", reference.meta.owner_id, path);
        // treat it as others
        owner.id = reference.meta.owner_id;
    }

    User current;
    if (!GetUserFromProcess(reference.owner_process, &current))
    {
        LOG("User from process %lu does not exist.\n", reference.owner_process)
        return -E_NO_PERMISSION;
    }

    bool user_executable = reference.meta.permissions & USER_EXECUTE;
    bool user_writable = reference.meta.permissions & USER_WRITE;
    bool user_readable = reference.meta.permissions & USER_READ;
    bool other_executable = reference.meta.permissions & OTHER_EXECUTE;
    bool other_writable = reference.meta.permissions & OTHER_WRITE;
    bool other_readable = reference.meta.permissions & OTHER_READ;

    if (current.id == owner.id)
    {
        if ((!user_executable && mode & FILE_EXECUTE) ||
            (!user_readable && mode & FILE_READ) ||
            (!user_writable && mode & FILE_WRITE))
        {
            LOG("User %lu (marked as owner) tried accessing file '%s'.\n", current.id, path);
            return -E_NO_PERMISSION;
        }
    }
    else
    {
        if ((!other_executable && mode & FILE_EXECUTE) ||
            (!other_readable && mode & FILE_READ) ||
            (!other_writable && mode & FILE_WRITE))
        {
            LOG("User %lu tried accessing file '%s' with owner %lu.\n", current.id, path, owner.id);
            return -E_NO_PERMISSION;
        }
    }

    return AddFileReference(reference);
}

void CloseFile(FILEHANDLE handle)
{
    RemoveFileReference(handle);
}

long ReadFile(FILEHANDLE handle, void *buf, unsigned long size, unsigned long offset)
{
    MountPoint mount;
    FileReference reference;

    if (!GetFileReference(handle, &reference))
        return -E_NO_FILE;

    if (!GetFileMount(handle, &mount))
        return -E_NO_FILE;

    if (mount.fs.fread == 0)
        return -E_INVALID_OPERATION;

    return mount.fs.fread(reference.cut_path, buf, size, offset, reference.meta.file_data, mount.key);
}

long WriteFile(FILEHANDLE handle, const char *buf, unsigned long size, unsigned long offset)
{
    MountPoint mount;
    FileReference reference;

    if (!GetFileReference(handle, &reference))
        return -E_NO_FILE;

    if (!GetFileMount(handle, &mount))
        return -E_NO_FILE;

    if (mount.fs.fwrite == 0)
        return -E_INVALID_OPERATION;

    return mount.fs.fwrite(reference.cut_path, buf, size, offset, reference.meta.file_data, mount.key);
}

long GetFileSize(const char *path)
{
    MountPoint mount;
    char *extra;
    if (!GetMountPoint(path, &mount, &extra))
        return -E_NO_MOUNT;

    if (mount.fs.fsize == 0)
    {
        free(extra);
        return -E_INVALID_OPERATION;
    }

    long ret = mount.fs.fsize(extra, mount.key);

    free(extra);
    return ret;
}

const char *GetFilename(const char *path)
{
    const char *last = strrchr(path, '/');
    if (last)
        return last + 1;
    else
        return path;
}