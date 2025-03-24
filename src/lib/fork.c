#include <lib/fork.h>
#include <scheduler.h>
#include <mm.h>
#include <stdbool.h>
#include <lib/printf.h>

bool fork(unsigned long fn, unsigned long arg, long kernel)
{
    preempt_disable();
    struct task *task = (struct task *)valloc(PAGE_SIZE_BYTES);
    if (!task)
        return false;

    struct pt_regs *childregs = task_pt_regs(task);
    qmemzero(childregs);
    qmemzero(&task->cpu_context);

    task->kernel = kernel;
    task->next = 0;
    task->state = TASK_RUNNING;
    task->counter = QUANTUM;
    task->preempt_count = 1;

    task->cpu_context.x19 = fn;
    task->cpu_context.x20 = arg;
    task->cpu_context.pc = (unsigned long)return_from_fork;
    task->cpu_context.sp = (unsigned long)childregs;
    add_task(task, false);
    preempt_enable();

    return true;
}

int move_to_user_mode(unsigned long pc)
{
    struct pt_regs *regs = task_pt_regs(scheduler_current);
    printf("moving to user mode from pc %lu to %lu\n", regs->pc, pc);
    qmemzero(regs);

    regs->pc = pc;
    regs->pstate = PSR_MODE_EL0t;

    unsigned long stack = valloc(PAGE_SIZE_BYTES); // allocate new user stack
    if (!stack)
        return false;
    regs->sp = stack + PAGE_SIZE;

    return true;
}

struct pt_regs *task_pt_regs(struct task *tsk)
{
    unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
    return (struct pt_regs *)p;
}