#include <svc.h>
#include <portal.h>

int close(long file)
{
    return (int)portal_request(PORTAL_VFS, 0, (void *)file, 1, 0);
}