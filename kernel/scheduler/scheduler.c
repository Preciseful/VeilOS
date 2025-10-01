#include <scheduler/scheduler.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <bundles/elf.h>

// dont use lib/list.h here as that allocates more

#define EL1H_M 0b0101

Task default_task = {0};
Task *scheduler_current;
unsigned long last_asid_chunk;
long last_pid;
bool stop;

void printx(unsigned long x)
{
    LOG("x: %u\n", x);
}

Task *GetRunningTask()
{
    return scheduler_current;
}

long AddTask(Task *task)
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

    last_pid++;
    return last_pid + 1;
}

void save_registers(Task *task, TaskRegs *registers)
{
    if (registers == 0)
        return;
    memcpy(&task->regs, registers, sizeof(task->regs));
}

void switch_task(Task *next)
{
    if (scheduler_current == next)
        return;

    scheduler_current = next;

    bool flush = false;
    unsigned long baddr = VIRT_TO_PHYS(next->mmu_ctx.pgd) & TTBR_BADDR_MASK;
    unsigned long asid = (unsigned long)next->mmu_ctx.asid << 48;

    if (last_asid_chunk != next->mmu_ctx.asid_chunk)
        flush = true;

    set_task_ttbr(asid | baddr, flush);

    last_asid_chunk = next->mmu_ctx.asid_chunk;
    stop = false;

    LOG("Switching to EL%u process: '%s' at 0x%lx.\n", next->kernel, next->name, next->mmu_ctx.va);
    cpu_switch_task(next);
}

void SchedulerInit()
{
    last_pid = 1;
    last_asid_chunk = 0;
    stop = true;
    scheduler_current = &default_task;
}

Task *get_next_task()
{
    Task *start = scheduler_current;
    Task *current = start;

    do
    {
        if (current->next && (current->next->flags & KILL_TASK))
        {
            current->next = current->next->next;
            KillTask(current->next);
        }

        current = current->next ? current->next : default_task.next;
        if (current == 0)
            return 0;

        if ((current->flags & ACTIVE_TASK) && !(current->flags & KILL_TASK))
            break;
    } while (current != start);

    return current;
}

void Schedule()
{
    Task *task = get_next_task();
    if (task)
        switch_task(task);
}

void SchedulerTick(TaskRegs *registers)
{
    save_registers(scheduler_current, registers);

    if (stop)
        return;

    scheduler_current->time--;
    if (scheduler_current->time > 0)
        return;

    scheduler_current->time = DEFAULT_TIME;
    stop = true;

    registers->elr_el1 = (unsigned long)&Schedule;
    registers->spsr_el1 = EL1H_M;
}

SYSCALL_HANDLER(exit_process)
{
    scheduler_current->flags |= KILL_TASK;
    scheduler_current->time = 0;
    return 1;
}