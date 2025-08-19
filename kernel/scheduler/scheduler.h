#pragma once

#include <stdbool.h>
#include <memory/mmu.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10
#define DEFAULT_TIME 5

typedef struct TaskRegs
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
} TaskRegs;

typedef struct TaskMapping
{
    VirtualAddr code;
    VirtualAddr va;
    PhysicalAddr pa;
} TaskMapping;

typedef struct TaskMMUCtx
{
    unsigned long *pgd;
    VirtualAddr sp_alloc;
    VirtualAddr va;
    PhysicalAddr pa;
} TaskMMUCtx;

typedef struct Task
{
    TaskRegs regs;
    TaskMMUCtx mmu_ctx;
    TaskMapping *mappings;
    unsigned long mappings_length;
    struct Task *next;

    const char *name;
    unsigned long flags;
    long time;
} Task;

void SchedulerInit();
void Schedule();
void AddTask(Task *task);
void SchedulerTick(unsigned long *stack);
Task *GetRunningTask();
extern void cpu_switch_task(Task *prev, Task *next, unsigned long *sp);
extern void set_task_ttbr(unsigned long *pgd);
