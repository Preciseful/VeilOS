#include <system/vfs.h>
#include <interface/errno.h>
#include <scheduler/scheduler.h>

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
    reference.owner = GetCurrentPID();
    reference.mode = mode;
    reference.path = path;

    MountPoint mount;
    char *extra;
    GetMountPoint(path, &mount, &extra);

    if (mount.fs.fopen)
    {
        int ret = mount.fs.fopen(extra, mode, mount.key);
        free(extra);

        if (ret != 0)
            return ret;

        return AddFileReference(reference);
    }
    else
    {
        free(extra);
        return -E_INVALID_OPERATION;
    }
}

void CloseFile(FILEHANDLE handle)
{
    RemoveFileReference(handle);
}

int ReadFile(FILEHANDLE handle, void *buf, unsigned long size, unsigned long offset)
{
    MountPoint mount;
    FileReference *reference;

    if (!GetFileReference(handle, &reference))
        return -E_NO_FILE;
    GetFileMount(handle, &mount);

    if (mount.fs.fread == 0)
        return -E_INVALID_OPERATION;

    return mount.fs.fread(reference->cut_path, reference->mode, buf, size, offset, mount.key);
}

int WriteFile(FILEHANDLE handle, const char *buf, unsigned long size, unsigned long offset)
{
    MountPoint mount;
    FileReference *reference;

    if (!GetFileReference(handle, &reference))
        return -E_NO_FILE;
    GetFileMount(handle, &mount);

    if (mount.fs.fwrite == 0)
        return -E_INVALID_OPERATION;

    return mount.fs.fwrite(reference->cut_path, reference->mode, buf, size, offset, mount.key);
}