#include <errno.h>
#undef errno
extern int errno;

void *sbrk(int incr)
{
    extern int errno;
    errno = ENOMEM;
    return (void *)-1;
}
