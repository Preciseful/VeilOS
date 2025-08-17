#pragma once

#include <scheduler/scheduler.h>
#include <memory/mmu.h>

task_t *pcreate(const char *name, unsigned long va, void *code);
void map_task_page(task_t *task, unsigned long va, enum MMU_Flags flags, void *code, unsigned long code_len);