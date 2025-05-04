#ifndef SCHEDULER_H
#define SCHEDULER_H

#define THREAD_CPU_CONTEXT 0

#ifndef __ASSEMBLER__

// clang-format off

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <iodevice.h>

#define THREAD_SIZE 4096
#define NUMBER_OF_TASKS 64

#define TASK_RUNNING 0
#define TASK_POINT 1

#define NR_TASKS 64

#define QUANTUM 5

#define KERNEL_FLAG 1
#define SHELL_FLAG 2

struct cpu_context
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long sp;
    unsigned long pc;
};

struct task
{
    struct cpu_context cpu_context;
    long state;
    long counter;
    struct task *next;
    long preempt_count;
    unsigned long flags;
    struct task *parent;
    unsigned long pid;
    struct io_device *io;
};

extern void cpu_switch_task(struct task *prev, struct task *next);

void set_pc(unsigned long pc);
void preempt_disable();
void preempt_enable();
void scheduler_init();
void schedule();
void scheduler_tick(unsigned int counter, unsigned int multiplier);
void scheduler_move_next();
unsigned long add_task(struct task *task, bool high);
void set_stack(unsigned long *st);

extern struct task *scheduler_current;

#ifdef __cplusplus
}
#endif

#endif
#endif