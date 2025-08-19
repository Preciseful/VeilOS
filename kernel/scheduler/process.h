#pragma once

#include <scheduler/scheduler.h>
#include <memory/mmu.h>

bool TaskContainsVA(Task *task, VirtualAddr va);
Task *PCreate(const char *name, VirtualAddr va, VirtualAddr code);
void MapTaskPage(Task *task, VirtualAddr va, enum MMU_Flags flags, VirtualAddr code, unsigned long code_len);
void UnmapTaskPage(Task *task, VirtualAddr va, unsigned long length);