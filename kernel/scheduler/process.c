#include <scheduler/process.h>
#include <memory/memory.h>
#include <lib/printf.h>

task_t *pcreate(void *pc)
{
    task_t *task = malloc(sizeof(task_t));
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = (unsigned long)pc;
    task->regs.sp = (unsigned long)task + PAGE_SIZE;

    add_task(task);
    return task;
}