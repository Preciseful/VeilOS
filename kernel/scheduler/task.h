#pragma once

#include <memory/mmu.h>
#include <lib/list.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10
#define KILL_MARK 0b100

typedef struct TaskRegs
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long x29;
    unsigned long x30;
    unsigned long sp;
    unsigned long sp_el1;
    unsigned long spsr;
    unsigned long elr;
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