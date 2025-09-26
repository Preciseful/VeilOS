#include <svc.h>
#include <errno.h>
#undef errno
extern int errno;

int execve(char *path, char **argv, char **env)
{
    if (!svc_call(path, argv, env, 0, 0, 0, 0, 0, SYS_EXECVE))
    {
        errno = ENOENT;
        return -1;
    }

    _exit(0);
}