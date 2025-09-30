#pragma once

#include <memory/mmu.h>
#include <lib/list.h>

#define USER_TASK 0b1
#define ACTIVE_TASK 0b10
#define KILL_TASK 0b100

typedef struct TaskRegs
{
    unsigned long x[28];
    unsigned long task_sp, interrupt_sp;
    unsigned long elr_el1, spsr_el1;
    unsigned long x28, x29, x30;
} TaskRegs;

enum Task_Mapping_Properties
{
    MAP_PROPERTY_CODE = 0b1,
    MAP_PROPERTY_VA = 0b10,
    MAP_PROPERTY_PA = 0b100
};

typedef struct TaskMapping
{
    VirtualAddr code;
    VirtualAddr va;
    PhysicalAddr pa;
    enum Task_Mapping_Properties to_free;
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
    long pid;
} Task;

bool TaskContainsVA(Task *task, VirtualAddr va);
Task *CreateTask(const char *name, bool kernel, VirtualAddr va, VirtualAddr code, char **environ, char **argv, int argc);
void MapTaskPage(Task *task, VirtualAddr va, enum MMU_Flags flags, VirtualAddr code, unsigned long code_len,
                 enum Task_Mapping_Properties properties_to_free);
void UnmapTaskPage(Task *task, VirtualAddr va, unsigned long length);
void KillTask(Task *task);

SYSCALL_HANDLER(execve);