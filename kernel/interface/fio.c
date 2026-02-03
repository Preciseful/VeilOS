#include <system/vfs.h>
#include <interface/errno.h>

FILEHANDLE OpenFile(PID pid, FILEMODE mode, const char *path)
{
    FileReference reference;
    reference.owner = pid;
    reference.mode = mode;
    reference.path = path;

    MountPoint point;
    char *extra;
    GetMountPoint(path, &point, &extra);

    if (point.fs.fopen)
    {
        point.fs.fopen(extra);
        return AddFileReference(reference);
    }
    else
        return -1;
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
        return -E_NOFILE;
    GetFileMount(handle, &mount);

    return mount.fs.fread(reference->cut_path, buf, size, offset, mount.key);
}