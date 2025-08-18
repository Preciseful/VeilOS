#pragma once

#include <scheduler/scheduler.h>
#include <memory/mmu.h>

bool task_contains_va(task_t *task, unsigned long va);
task_t *pcreate(const char *name, unsigned long va, void *code);
void map_task_page(task_t *task, unsigned long va, enum MMU_Flags flags, void *code, unsigned long code_len);
void unmap_task_page(task_t *task, unsigned long va, unsigned long length);