#include <svc.h>
#include <errno.h>
#include <newlib/newlib.h>
#undef errno
extern int errno;

int execve(char *path, char **argv, char **env)
{
    if (!svc_call((unsigned long)path, (unsigned long)argv, (unsigned long)env, 0, 0, 0, 0, 0, SYS_EXECVE))
    {
        errno = ENOENT;
        return -1;
    }

    _exit(0);
}