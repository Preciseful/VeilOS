#pragma once

#include <memory/mmu.h>
#include <lib/list.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10
#define KILL_TASK 0b100

typedef long PID;

typedef struct TaskRegs
{
    unsigned long x[28];
    unsigned long task_sp, interrupt_sp;
    unsigned long elr_el1, spsr_el1;
    unsigned long x28, x29, x30;
} TaskRegs;

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
    TaskRegs regs;
    TaskMMUCtx mmu_ctx;
    // denotes va->pa relations with a radix tree
    TaskMappingNode *map_root;
    struct Task *next;

    char *name;
    bool kernel;
    int argc;
    int environc;
    // the values within argv are user allocated, accesses must be done with PHYS_TO_VIRT
    char **argv;
    // the values within environ are user allocated, accesses must be done with PHYS_TO_VIRT
    char **environ;
    unsigned long flags;
    long time;
    PID pid;
} Task;

Task *CreateTask(const char *name, bool kernel, VirtualAddr va, PhysicalAddr data_pa, char **environ, char **argv, int argc);
PhysicalAddr GetPagePA(Task *task, VirtualAddr va);
void MapTaskPage(Task *task, VirtualAddr va, PhysicalAddr pa, unsigned int size, enum MMU_Flags flags);
void UnmapTaskPage(Task *task, VirtualAddr va, unsigned int length);
void RemoveMapsFromNode(Task *task, TaskMappingNode *node);
void KillTask(Task *task);
VirtualAddr GetTaskValidVA(Task *task, unsigned int size);

SYSCALL_HANDLER(execve);