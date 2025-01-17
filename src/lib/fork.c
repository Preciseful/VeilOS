#include <lib/fork.h>
#include <scheduler.h>
#include <mm.h>
#include <stdbool.h>

bool fork(unsigned long fn, unsigned long arg, long kernel)
{
    preempt_disable();
    struct task *task = (struct task *)valloc(PAGE_SIZE_BYTES);
    if (!task)
        return false;

    task->kernel = kernel;
    task->next = 0;
    task->state = TASK_RUNNING;
    task->counter = QUANTUM;
    task->preempt_count = 1;

    task->cpu_context.x19 = fn;
    task->cpu_context.x20 = arg;
    task->cpu_context.pc = (unsigned long)return_from_fork;
    task->cpu_context.sp = (unsigned long)task + THREAD_SIZE;
    add_task(task, false);
    preempt_enable();

    return true;
}