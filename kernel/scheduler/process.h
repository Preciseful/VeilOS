#pragma once

#include <scheduler/scheduler.h>
#include <memory/mmu.h>

bool task_contains_va(task_t *task, virtual_addr va);
task_t *pcreate(const char *name, virtual_addr va, virtual_addr code);
void map_task_page(task_t *task, virtual_addr va, enum MMU_Flags flags, virtual_addr code, unsigned long code_len);
void unmap_task_page(task_t *task, virtual_addr va, unsigned long length);