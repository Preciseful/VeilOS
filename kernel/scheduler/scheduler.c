#include <scheduler/scheduler.h>
#include <lib/printf.h>
#include <memory/mmu.h>

task_t default_task = {0};
task_t *scheduler_current;
bool stop = true;

void printx(unsigned long x)
{
    printf("x: %lu\n", x);
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

void switch_task(task_t *next)
{
    if (scheduler_current == next)
        return;

    task_t *last = scheduler_current;
    scheduler_current = next;

    stop = false;
    cpu_switch_task(last, next, next->pgd, 0);
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
        if (current->flags & ACTIVE_TASK)
            break;
    } while (current != start);

    return current;
}

void schedule()
{
    task_t *current = get_next_task();
    switch_task(current);
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
    task_t *last = scheduler_current;
    scheduler_current = next;

    cpu_switch_task(last, next, next->pgd, stack);
}