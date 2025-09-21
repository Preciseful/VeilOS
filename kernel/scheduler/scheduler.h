#pragma once

#include <stdbool.h>
#include <memory/mmu.h>
#include <scheduler/task.h>

#define DEFAULT_TIME 5

void SchedulerInit();
Task *GetRunningTask();
void AddTask(Task *task);
void Schedule();
void SchedulerTick(unsigned long *stack);

SYSCALL_HANDLER(exit_process);

extern void cpu_switch_task(Task *prev, Task *next, unsigned long *sp);
extern void set_task_ttbr(unsigned long pgd, bool flush);
