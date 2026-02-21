#include <memory/mmu.h>
#include <memory/memory.h>
#include <scheduler/scheduler.h>
#include <drivers/uart.h>
#include <boot/interrupts.h>

typedef unsigned long (*SvcHandler)(InterruptStack *sp);

enum System_Calls
{
    SYS_MALLOC,
    SYS_FREE,
    SYS_GET_MEMORY_SIZE,
    SYS_EXIT_PROCESS,
};

static int svc_priority[] = {
    [SYS_MALLOC] = 1,
    [SYS_FREE] = 1,
    [SYS_GET_MEMORY_SIZE] = 1,
    [SYS_EXIT_PROCESS] = 1,
};

static SvcHandler svc_table[] = {
    [SYS_MALLOC] = SystemCall_malloc,
    [SYS_FREE] = SystemCall_free,
    [SYS_GET_MEMORY_SIZE] = SystemCall_memory_size,
    [SYS_EXIT_PROCESS] = SystemCall_exit_process,
};

void HandleSystemCall(InterruptStack *sp)
{
    unsigned long code = sp->x8;
    if (code >= sizeof(svc_table) / sizeof(svc_table[0]) || !svc_table[code])
    {
        sp->x0 = -1;
        return;
    }

    if (svc_priority[code] == 0)
        irq_enable();

    sp->x0 = svc_table[code](sp);
}
