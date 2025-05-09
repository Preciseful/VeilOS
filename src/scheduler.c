#include <scheduler.h>
#include <stdbool.h>
#include <interrupts.h>
#include <lib/printf.h>
#include <funcs.h>
#include <lib/fork.h>
#include <mm.h>

#define INIT_TASK                             \
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
     TASK_POINT,                              \
     0,                                       \
     0,                                       \
     0,                                       \
     KERNEL_FLAG,                             \
     0,                                       \
     1,                                       \
     0}

struct task high_task = INIT_TASK;
struct task low_task = INIT_TASK;
struct task *scheduler_current = &high_task;
unsigned long to_pc = 0;
unsigned long *stack = 0;

static unsigned long lows = 0, highs = 0;
static bool moved_next;

void printx(unsigned long x)
{
    printf("x: %lu\n", x);
}

void set_pc(unsigned long pc)
{
    to_pc = pc;
}

void set_stack(unsigned long *st)
{
    stack = st;
}

void preempt_disable()
{
    scheduler_current->preempt_count++;
}

void preempt_enable()
{
    scheduler_current->preempt_count--;
}

void switch_to(struct task *next)
{
    if (scheduler_current == next)
        return;
    struct task *prev = scheduler_current;
    scheduler_current = next;
    // printf("\ncurrent task: %s, counter: %ld, preempt %ld\n",
    //        (unsigned char *)scheduler_current->cpu_context.x20,
    //        scheduler_current->counter,
    //        scheduler_current->preempt_count);

    moved_next = true;
    // printf("next: %lu \n", next->cpu_context.pc);
    cpu_switch_task(prev, next);
}

unsigned long add_task(struct task *task, bool high)
{
    unsigned long i = 0;
    if (high)
    {
        struct task *c = &high_task;
        while (c->next && c->next != &low_task)
        {
            c = c->next;
            i++;
        }

        c->next = task;
        i++;
        task->next = &low_task;

        highs++;
    }
    else
    {
        struct task *c = &low_task;
        while (c->next && c->next != &high_task)
        {
            c = c->next;
            i++;
        }

        c->next = task;
        i++;
        task->next = &high_task;

        lows++;
    }

    return i;
}

struct task *next_running_task(struct task *current)
{
    struct task *c = current;
    while (c->next && c->next->state != TASK_RUNNING)
        c = c->next;

    c = c->next;
    return c;
}

void scheduler_init()
{
    high_task.next = &low_task;
    low_task.next = &high_task;
}

void schedule()
{
    preempt_disable();
    struct task *p = 0;

    while (true)
    {
        p = 0;

        for (struct task *c = high_task.next; c && c != &high_task; c = c->next)
        {
            if (c->state == TASK_RUNNING && c->counter > 0)
            {
                p = c;
                break;
            }
        }

        if (p)
            break;

        p = next_running_task(&high_task);
        if (p)
            p->counter = QUANTUM;
    }

    PINFO("\n", "Switching to: %lu\n\n", p->cpu_context.x19);
    if (p->flags & SHELL_FLAG)
        p->io->status = ACTIVE;

    switch_to(p);
    preempt_enable();
}

void scheduler_tick(unsigned int counter, unsigned int multiplier)
{
    // printf(".");
    scheduler_current->counter--;
    if (scheduler_current->counter > 0 || scheduler_current->preempt_count > 0)
        return;

    // printf("now");

    if (to_pc != 0)
    {
        struct pt_regs *regs = (struct pt_regs *)stack;
        // qmemzero(regs);

        regs->pc = to_pc;
        regs->pstate = PSR_MODE_EL0t;

        unsigned long stack = valloc(PAGE_SIZE_BYTES); // allocate new user stack
        if (!stack)
            return false;
        regs->sp = stack + PAGE_SIZE;
        asm("mov x0, %0\n"
            "msr sp_el0, x0"
            :
            : "r"(regs->sp)
            : "x0");

        to_pc = 0;
    }

    struct task *next = next_running_task(scheduler_current);
    next->counter = QUANTUM;

    // printf("\ncurrent task: %s, counter: %ld, preempt %ld",
    //        (unsigned char *)scheduler_current->cpu_context.x20,
    //        scheduler_current->counter,
    //        scheduler_current->preempt_count);

    // printf("\nnext task: %s, counter: %ld, preempt %ld\n",
    //        (unsigned char *)next->cpu_context.x20,
    //        next->counter,
    //        next->preempt_count);
    if (next != scheduler_current)
    {
        irq_enable();
        schedule();
        irq_disable();
    }
}

void scheduler_move_next()
{
    if (next_running_task(scheduler_current) == scheduler_current)
        return;
    moved_next = false;
    scheduler_current->counter = 0;
    scheduler_current->preempt_count = 0;
    while (!moved_next)
        ;
}