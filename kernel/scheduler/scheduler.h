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
    unsigned long sp_el1;
    unsigned long spsr;
    unsigned long elr;
} task_regs_t;

typedef struct task_mapping
{
    void *code;
    unsigned long va;
    unsigned long pa;
} task_mapping_t;

typedef struct task
{
    task_regs_t regs;
    const char *name;
    unsigned long flags;
    unsigned long *pgd;
    unsigned long phys_sp;
    unsigned long va;
    unsigned long pa;
    task_mapping_t *mappings;
    unsigned long mappings_length;
    long time;
    struct task *next;
} task_t;

void scheduler_init();
void schedule();
void add_task(task_t *task);
void scheduler_tick(unsigned long *stack);
task_t *get_running_task();
extern void cpu_switch_task(task_t *prev, task_t *next, unsigned long *sp);
extern void set_task_ttbr(unsigned long *pgd);
