#pragma once

#include <memory/mmu.h>
#include <lib/list.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10

typedef struct TaskRegs
{
    unsigned long x[28];
    unsigned long sp_el0, sp_el1;
    unsigned long elr_el1, spsr_el1;
    unsigned long x28, x29, x30;
} TaskRegs;

typedef struct TaskMapping
{
    VirtualAddr code;
    VirtualAddr va;
    PhysicalAddr pa;
} TaskMapping;

typedef struct TaskMMUCtx
{
    unsigned long asid_chunk;
    unsigned char asid;
    unsigned long *pgd;
    VirtualAddr sp_alloc;
    VirtualAddr va;
    PhysicalAddr pa;
} TaskMMUCtx;

typedef struct Task
{
    TaskRegs regs;
    TaskMMUCtx mmu_ctx;
    List mappings;
    struct Task *next;

    const char *name;
    unsigned long flags;
    long time;
} Task;

bool TaskContainsVA(Task *task, VirtualAddr va);
Task *CreateTask(const char *name, VirtualAddr va, VirtualAddr code);
void MapTaskPage(Task *task, VirtualAddr va, enum MMU_Flags flags, VirtualAddr code, unsigned long code_len);
void UnmapTaskPage(Task *task, VirtualAddr va, unsigned long length);