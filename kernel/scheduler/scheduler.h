/**
 * @file
 * @author Developful
 * @brief Scheduling processes interfacing.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <system/syscall.h>

/**
 * @brief The amount of time allocated to a task.
 */
#define DEFAULT_TIME 5

/**
 * @brief Initializes the scheduler.
 */
void SchedulerInit();

/**
 * @brief Starts scheduling.
 */
void Schedule();

/**
 * @brief Adds a task to scheduling.
 *
 * @param task The task to schedule.
 * @return The process id.
 */
PID AddTask(Task *task);

/**
 * @brief A scheduler tick, called whenever the timer ticks.
 *
 * @param registers The registers from the timer interrupt.
 */
void SchedulerTick(InterruptStack registers[]);

/**
 * @return Gets the currently running task.
 */
Task *GetRunningTask();

/**
 * @brief Gets a task by the process id.
 *
 * @param pid The process id.
 * @param[out] btask A pointer to the task.
 * @return `true` if found, otherwise `false`.
 */
bool GetTaskByPID(PID pid, Task **btask);

/**
 * @return The currently running task's process id.
 */
PID GetCurrentPID();

SYSCALL_HANDLER(exit_process);