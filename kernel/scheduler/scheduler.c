#include <scheduler/scheduler.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/process.h>

Task default_task = {0};
Task *scheduler_current;
bool stop = true;

void printx(unsigned long x)
{
    LOG("x: %lu\n", x);
}

void AddTask(Task *task)
{
    if (!default_task.next)
    {
        default_task.next = task;
        task->next = task;
    }
    else
    {
        Task *last = default_task.next;
        while (last->next && last->next != default_task.next)
            last = last->next;

        last->next = task;
        task->next = default_task.next;
    }
}

void switch_task(Task *next, unsigned long *stack)
{
    if (scheduler_current == next)
        return;

    Task *last = scheduler_current;
    scheduler_current = next;

    set_task_ttbr(next->mmu_ctx.pgd);
    stop = false;
    LOG("Switching to process: '%s' at 0x%lx.\n", next->name, next->mmu_ctx.va);
    cpu_switch_task(last, next, stack);
}

void SchedulerInit()
{
    scheduler_current = &default_task;
}

Task *get_next_task()
{
    Task *start = scheduler_current;
    Task *current = start;

    do
    {
        current = current->next ? current->next : default_task.next;
        if (current == 0)
            return 0;

        if (current->flags & ACTIVE_TASK)
            break;
    } while (current != start);

    return current;
}

void Schedule()
{
    Task *current = get_next_task();
    if (current)
        switch_task(current, 0);
}

void SchedulerTick(unsigned long *stack)
{
    if (stop)
        return;

    scheduler_current->time--;
    if (scheduler_current->time > 0)
        return;

    scheduler_current->time = DEFAULT_TIME;
    Task *next = get_next_task();
    switch_task(next, stack);
}

Task *GetRunningTask()
{
    return scheduler_current;
}