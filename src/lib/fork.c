#include <lib/fork.h>
#include <scheduler.h>
#include <mm.h>
#include <stdbool.h>
#include <lib/printf.h>
#include <drivers/miniuart.h>

bool clone()
{
    preempt_disable();
    unsigned long fn = scheduler_current->cpu_context.x19;
    unsigned long arg = scheduler_current->cpu_context.x20;
    unsigned long flags = scheduler_current->flags;

    bool ret = fork(fn, arg, flags, scheduler_current);
    preempt_enable();
    return ret;
}

struct pt_regs *task_pt_regs(struct task *tsk)
{
    unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
    return (struct pt_regs *)p;
}

int move_task_to_user_mode(struct task *x)
{
    struct pt_regs *regs = task_pt_regs(x);
    INFO("regs: %lu\n", regs);
    // qmemzero(regs);

    regs->pstate = PSR_MODE_EL0t;

    return true;
}

int move_to_user_mode(unsigned long pc)
{
    struct pt_regs *regs = task_pt_regs(scheduler_current);
    INFO("regs: %lu\n", regs);
    // qmemzero(regs);

    regs->pc = pc;
    regs->pstate = PSR_MODE_EL0t;

    return true;
}

bool fork(unsigned long fn, unsigned long arg, long flags, struct task *parent)
{
    preempt_disable();
    struct task *task = (struct task *)valloc(PAGE_SIZE_BYTES);
    if (!task)
        return false;

    struct pt_regs *childregs = task_pt_regs(task);
    qmemzero(childregs);
    qmemzero(&task->cpu_context);

    if (parent != 0)
    {
        memcpy(task, parent, sizeof(struct task));
        memcpy(childregs, task_pt_regs(parent), sizeof(struct pt_regs));
        childregs->regs[0] = 0;
    }
    else
    {
        task->flags = flags;
        task->next = 0;
        task->state = TASK_RUNNING;
        task->counter = QUANTUM;
        task->preempt_count = 1;
        task->io = qalloc(struct io_device);
        task->io->index = 0;
        task->io->put = uart_put;
        task->io->read = uart_readchar;
        task->io->status = INACTIVE;
    }

    task->parent = parent;
    task->cpu_context.x19 = fn;
    task->cpu_context.x20 = arg;
    task->cpu_context.pc = (unsigned long)return_from_fork;
    task->cpu_context.sp = (unsigned long)childregs;

    if (!(task->flags & KERNEL_FLAG))
        move_task_to_user_mode(task->cpu_context.pc);

    unsigned long pid = add_task(task, false);
    if (parent != 0)
        pid = 0;
    task->pid = pid;
    preempt_enable();

    return true;
}