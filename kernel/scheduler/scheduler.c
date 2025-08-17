#include <scheduler/scheduler.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/process.h>

task_t default_task = {0};
task_t *scheduler_current;
bool stop = true;

void printx(unsigned long x)
{
    LOG("x: %lu\n", x);
}

void add_task(task_t *task)
{
    if (!default_task.next)
    {
        default_task.next = task;
        task->next = task;
    }
    else
    {
        task_t *last = default_task.next;
        while (last->next && last->next != default_task.next)
            last = last->next;

        last->next = task;
        task->next = default_task.next;
    }
}

void switch_task(task_t *next, unsigned long *stack)
{
    if (scheduler_current == next)
        return;

    task_t *last = scheduler_current;
    scheduler_current = next;

    set_task_ttbr(next->pgd);
    stop = false;

    cpu_switch_task(last, next, stack);
}

void scheduler_init()
{
    scheduler_current = &default_task;
}

task_t *get_next_task()
{
    task_t *start = scheduler_current;
    task_t *current = start;

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

void schedule()
{
    task_t *current = get_next_task();
    if (current)
        switch_task(current, 0);
}

void scheduler_tick(unsigned long *stack)
{
    if (stop)
        return;

    scheduler_current->time--;
    if (scheduler_current->time > 0)
        return;

    scheduler_current->time = DEFAULT_TIME;
    task_t *next = get_next_task();
    switch_task(next, stack);
}