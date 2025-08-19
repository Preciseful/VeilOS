#pragma once

#include <scheduler/scheduler.h>
#include <memory/mmu.h>

bool task_contains_va(task_t *task, VirtualAddr va);
task_t *pcreate(const char *name, VirtualAddr va, VirtualAddr code);
void map_task_page(task_t *task, VirtualAddr va, enum MMU_Flags flags, VirtualAddr code, unsigned long code_len);
void unmap_task_page(task_t *task, VirtualAddr va, unsigned long length);