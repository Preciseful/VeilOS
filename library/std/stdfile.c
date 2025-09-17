#include <std/stdfile.h>
#include <portal.h>

#define VFS_REQUEST_OPEN 0
#define VFS_REQUEST_CLOSE 1
#define VFS_REQUEST_SEEK 2

FileID OpenFile(const char *path)
{
    return portal_request(PORTAL_VFS, 0, 0, VFS_REQUEST_OPEN, path);
}

void CloseFile(FileID id)
{
    portal_request(PORTAL_VFS, 0, (void *)id, VFS_REQUEST_CLOSE, 0);
}

unsigned long ReadFromFile(FileID id, void *buf, unsigned long size)
{
    return portal_read(PORTAL_VFS, 0, (void *)id, buf, size);
}

unsigned long WriteToFile(FileID id, void *buf, unsigned long size)
{
    return portal_write(PORTAL_VFS, 0, (void *)id, buf, size);
}

void SeekInFile(FileID id, unsigned long seek)
{
    portal_request(PORTAL_VFS, 0, (void *)id, VFS_REQUEST_SEEK, (void *)seek);
}
