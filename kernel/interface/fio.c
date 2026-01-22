#include <system/vfs.h>

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