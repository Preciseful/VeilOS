#pragma once

#include <memory/mmu.h>
#include <lib/list.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10
#define KILL_TASK 0b100

typedef long PID;

typedef struct TaskMMUCtx
{
    unsigned long asid_chunk;
    unsigned char asid;
    unsigned long *pgd;
    VirtualAddr sp_alloc;
    VirtualAddr va;
    PhysicalAddr pa;
} TaskMMUCtx;

typedef struct TaskMappingNode
{
    struct TaskMappingNode *children[0xFF];
    struct TaskMappingNode *parent;
    PhysicalAddr pa;
    VirtualAddr va;
    bool leaf;
    unsigned long full_children;
} TaskMappingNode;

typedef struct Task
{
    InterruptStack regs;
    TaskMMUCtx mmu_ctx;
    // denotes va->pa relations with a radix tree
    TaskMappingNode *map_root;
    struct Task *next;

    char *name;
    bool kernel;
    unsigned long flags;
    long time;
    PID pid;
} Task;

Task *CreateTask(const char *name, bool kernel, VirtualAddr va, PhysicalAddr data_pa);
PhysicalAddr GetPagePA(Task *task, VirtualAddr va);
void MapTaskPage(Task *task, VirtualAddr va, PhysicalAddr pa, unsigned int size, enum MMU_Flags flags);
void UnmapTaskPage(Task *task, VirtualAddr va, unsigned int length);
void RemoveMapsFromNode(Task *task, TaskMappingNode *node);
void KillTask(Task *task);
VirtualAddr GetTaskValidVA(Task *task, unsigned int size);
