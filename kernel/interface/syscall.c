#include <memory/mmu.h>
#include <memory/memory.h>
#include <scheduler/scheduler.h>
#include <user/environment.h>
#include <drivers/uart.h>

typedef unsigned long (*SvcHandler)(unsigned long *sp);

enum System_Calls
{
    SYS_MALLOC,
    SYS_FREE,
    SYS_GET_MEMORY_SIZE,
    SYS_EXIT_PROCESS,
    SYS_SET_ENVIRON,
    SYS_EXECVE,
    SYS_PRINT,
};

static SvcHandler svc_table[] = {
    [SYS_MALLOC] = SystemCall_malloc,
    [SYS_FREE] = SystemCall_free,
    [SYS_GET_MEMORY_SIZE] = SystemCall_memory_size,
    [SYS_EXIT_PROCESS] = SystemCall_exit_process,
    [SYS_SET_ENVIRON] = SystemCall_get_environ,
};

void HandleSystemCall(unsigned long *sp)
{
    unsigned long code = sp[8];
    if (code >= sizeof(svc_table) / sizeof(svc_table[0]) || !svc_table[code])
    {
        sp[0] = -1;
        return;
    }

    sp[0] = svc_table[code](sp);
}
