#pragma once

#include <stdbool.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <system/syscall.h>

#define DEFAULT_TIME 5

void SchedulerInit();
void Schedule();
PID AddTask(Task *task);
void SchedulerTick(TaskRegs registers[]);
Task *GetRunningTask();
bool GetTaskByPID(PID pid, Task **btask);
PID GetCurrentPID();
extern void cpu_switch_task(Task *next);
extern void set_task_ttbr(unsigned long pgd, bool flush);

SYSCALL_HANDLER(exit_process);