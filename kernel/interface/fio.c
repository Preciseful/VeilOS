#include <system/vfs.h>

int OpenFile(const char *path)
{
    MountPoint point;
    char *extra;
    GetMountPoint(path, &point, &extra);

    if (point.fs.fopen)
        return point.fs.fopen(extra);
    else
        return -1;
}