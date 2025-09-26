#include <svc.h>

void _exit(int code)
{
    svc_call(code, 0, 0, 0, 0, 0, 0, 0, SYS_EXIT_PROCESS);
    while (1)
        ;
}