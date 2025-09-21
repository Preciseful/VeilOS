#include <errno.h>
#include <system/svc.h>
#undef errno
extern int errno;

void _exit()
{
    svc_call(0, 0, 0, 0, 0, 0, 0, 0, 4);
    while (1)
        ;
}