#pragma once

#include <stdbool.h>
#include <memory/mmu.h>
#include <scheduler/task.h>

#define DEFAULT_TIME 5

void SchedulerInit();
void Schedule();
void AddTask(Task *task);
void SchedulerTick(unsigned long *stack);
Task *GetRunningTask();
extern void cpu_switch_task(Task *next);
extern void set_task_ttbr(unsigned long pgd, bool flush);
