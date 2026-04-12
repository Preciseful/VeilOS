/**
 * @file
 * @author Developful
 * @brief Scheduler tasks interface.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory/mmu.h>

typedef long PID;

#include <system/user.h>

#define USER_TASK (1 << 0)
#define ACTIVE_TASK (1 << 1)
#define KILL_TASK (1 << 2)

#define EL0T_M 0b0000
#define EL1H_M 0b0101

typedef struct IODeviceOwnership IODeviceOwnership;

typedef struct TaskMMUCtx
{
    unsigned long asid_chunk;
    unsigned char asid;
    unsigned long *pgd;
    VirtualAddr sp_el0_kernel;
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
    UID uid;

    IODeviceOwnership *devices;
    unsigned long devices_count;
} Task;

Task *CreateTask(const char *name, bool kernel, VirtualAddr va, PhysicalAddr data_pa, int argc, char **argv, int envargc, char **envargv);
PhysicalAddr GetPagePA(Task *task, VirtualAddr va);
void MapTaskPage(Task *task, VirtualAddr va, PhysicalAddr pa, unsigned int size, enum MMU_Flags flags);
void UnmapTaskPage(Task *task, VirtualAddr va, unsigned int length);
void RemoveMapsFromNode(Task *task, TaskMappingNode *node);
void KillTask(Task *task);
VirtualAddr GetTaskValidVA(Task *task, unsigned int size);
