#include <scheduler/process.h>
#include <memory/memory.h>
#include <lib/printf.h>

task_t *pcreate(unsigned long pc)
{
    printf("1");
    task_t *task = malloc(sizeof(task_t));
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = pc;
    task->regs.sp = (unsigned long)task + PAGE_SIZE;

    printf("2");
    add_task(task);

    printf("3");
    return task;
}