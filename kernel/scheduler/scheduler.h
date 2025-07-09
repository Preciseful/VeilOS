#pragma once

#include <stdbool.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10
#define DEFAULT_TIME 5

typedef struct task_regs
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
    unsigned long x29;
    unsigned long x30;
    unsigned long sp;
} task_regs_t;

typedef struct task
{
    task_regs_t regs;
    unsigned long flags;
    long time;
    struct task *next;
    unsigned long *pgd;
} task_t;

void scheduler_init();
void schedule();
void add_task(task_t *task);
void scheduler_tick(unsigned long *stack);
extern void cpu_switch_task(task_t *prev, task_t *next, unsigned long *table);
extern void cpu_switch_from_tick(task_t *prev, task_t *next, unsigned long *table);