#include <user/environment.h>
#include <scheduler/scheduler.h>

SYSCALL_HANDLER(set_environ)
{
    char ***environ = (char ***)sp[0];
    *environ = (char **)VIRT_TO_PHYS(GetRunningTask()->environ);
    return 0;
}