#include <errno.h>
#include <system/svc.h>
#undef errno
extern int errno;

void _exit_()
{
    svc_call(0, 0, 0, 0, 0, 0, 0, 0, 4);
}