#include <scheduler/task.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/scheduler.h>
#include <lib/string.h>
#include <bundles/elf.h>
#include <lib/panic.h>

#define EL0T_M 0b0000
#define EL1H_M 0b0101
#define ASID_CHUNKS_NUMBER 256

#define MAP_BITS (sizeof(unsigned char) * 8)
#define MAPPING_LEVELS (sizeof(unsigned long) / (MAP_BITS / 8))
#define MAPPING_VALUE (0xFF)

extern unsigned char el1_vectors[];

Task *asid_chunks[ASID_CHUNKS_NUMBER][256] = {};

TaskMappingNode *create_map_node(TaskMappingNode *parent)
{
    TaskMappingNode *node = (TaskMappingNode *)malloc(sizeof(TaskMappingNode));
    memset(node, 0, sizeof(TaskMappingNode));
    node->parent = parent;

    return node;
}

PhysicalAddr GetPagePA(Task *task, VirtualAddr va)
{
    unsigned short va_parts[MAPPING_LEVELS];
    for (int i = 0; i < MAPPING_LEVELS; i++)
        va_parts[i] = (va >> (i * MAP_BITS)) & MAPPING_VALUE;

    TaskMappingNode *node = task->map_root;
    for (unsigned long i = 0; i < MAPPING_LEVELS; i++)
    {
        if (!node->children[va_parts[i]])
            return 0;

        node = node->children[va_parts[i]];
    }

    return node->pa;
}

void MapTaskPage(Task *task, VirtualAddr va, PhysicalAddr pa, unsigned int size, enum MMU_Flags flags)
{
    unsigned long num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (unsigned long p = 0; p < num_pages; p++)
    {
        unsigned short va_parts[MAPPING_LEVELS];
        for (int i = 0; i < MAPPING_LEVELS; i++)
            va_parts[i] = (va >> (i * MAP_BITS)) & MAPPING_VALUE;

        TaskMappingNode *node = task->map_root;
        for (unsigned long i = 0; i < MAPPING_LEVELS; i++)
        {
            if (!node->children[va_parts[i]])
                node->children[va_parts[i]] = create_map_node(node);

            node = node->children[va_parts[i]];
        }

        if (node->leaf && node->pa != pa && node->pa != 0)
            UnmapTablePage(task->mmu_ctx.pgd, va);

        TaskMappingNode *parent = node->parent;

        while (parent)
        {
            parent->full_children++;
            if (parent->full_children < MAPPING_VALUE)
                break;

            parent->full_children = MAPPING_VALUE;
            parent = parent->parent;
        }

        node->leaf = true;
        node->pa = pa;
        node->va = va;

        MapTablePage(task->mmu_ctx.pgd, va, pa, MAIR_IDX_NORMAL, flags);

        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }
}

void UnmapTaskPage(Task *task, VirtualAddr va, unsigned int length)
{
    if (!GetPagePA(task, va))
        return;

    unsigned long num_pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned long i = 0; i < num_pages; i++)
    {
        unsigned long offset = i * PAGE_SIZE;
        va += offset;

        unsigned short va_parts[MAPPING_LEVELS];
        for (int i = 0; i < MAPPING_LEVELS; i++)
            va_parts[i] = (va >> (i * MAP_BITS)) & MAPPING_VALUE;

        TaskMappingNode *node = task->map_root;
        for (unsigned long i = 0; i < MAPPING_LEVELS; i++)
        {
            // something def went wrong
            if (node == 0)
            {
                panic("Error occured: node is 0.");
                return;
            }

            node = node->children[va_parts[i]];
        }

        TaskMappingNode *parent = node->parent;
        parent->children[va_parts[MAPPING_LEVELS - 1]] = 0;

        while (parent)
        {
            parent->full_children--;

            if (parent->full_children == MAPPING_VALUE - 1)
                parent = parent->parent;
            else
                break;
        }

        free(node);
        UnmapTablePage(task->mmu_ctx.pgd, va);
    }
}

void RemoveMapsFromNode(Task *task, TaskMappingNode *node)
{
    for (unsigned long i = 0; i < MAPPING_VALUE; i++)
    {
        if (!node->children[i])
            continue;

        if (node->children[i]->leaf)
        {
            UnmapTablePage(task->mmu_ctx.pgd, node->children[i]->va);
            free(node->children[i]);
            node->children[i] = 0;
        }
        else
            RemoveMapsFromNode(task, node->children[i]);
    }

    free(node);
}

void KillTask(Task *task)
{
    RemoveMapsFromNode(task, task->map_root);

    free(task->name);
    FreeTable(task->mmu_ctx.pgd, 0);
    free((void *)task->mmu_ctx.pa);
    free(task);
}

VirtualAddr GetTaskValidVA(Task *task, unsigned int size)
{
    unsigned long va = 0;

    TaskMappingNode *node = task->map_root;
    for (unsigned long level = 0; level < MAPPING_LEVELS - 1; level++)
    {
        for (unsigned long i = 0; i < MAPPING_VALUE; i++)
        {
            TaskMappingNode *child = node->children[i];

            if (!child)
            {
                va += i << ((MAPPING_LEVELS - 1 - level) * MAP_BITS + PAGE_SHIFT);
                return va;
            }

            if (child->full_children == MAPPING_VALUE)
                continue;

            va += i << ((MAPPING_LEVELS - 1 - level) * MAP_BITS + PAGE_SHIFT);
            node = child;
            break;
        }
    }

    for (unsigned long i = 1; i < MAPPING_VALUE; i++)
    {
        if (node->children[i])
            continue;

        va += i << PAGE_SHIFT;
        return va;
    }

    return 0;
}

void *mapped_malloc(Task *task, unsigned int size)
{
    void *data = malloc(size);
    MapTaskPage(task, GetTaskValidVA(task, size), VIRT_TO_PHYS(data), size, MMU_USER | MMU_RWRW);
    return data;
}

void push_args(Task *task, int argc, char **argv)
{
    unsigned long user_argv[argc];

    for (int i = argc - 1; i >= 0; i--)
    {
        unsigned long len = strlen(argv[i]) + 1;
        task->mmu_ctx.sp_el0_kernel -= len;
        task->regs.sp_el0 -= len;

        memcpy((char *)task->mmu_ctx.sp_el0_kernel, argv[i], len);
        user_argv[i] = task->regs.sp_el0;
    }

    task->mmu_ctx.sp_el0_kernel &= ~0xF;
    task->regs.sp_el0 &= ~0xF;

    for (int i = argc - 1; i >= 0; i--)
    {
        task->mmu_ctx.sp_el0_kernel -= sizeof(unsigned long);
        task->regs.sp_el0 -= sizeof(unsigned long);
        *(unsigned long *)task->mmu_ctx.sp_el0_kernel = user_argv[i];
    }
}

Task *CreateTask(const char *name, bool kernel, VirtualAddr va, PhysicalAddr data_pa, int argc, char **argv, int envargc, char **envargv)
{
    Task *task = malloc(sizeof(Task));
    memset(&task->regs, 0, sizeof(task->regs));

    unsigned long name_len = strlen(name) + 1;
    task->name = malloc(name_len);
    memcpy(task->name, name, name_len);

    task->flags = ACTIVE_TASK;
    task->kernel = kernel;
    task->time = DEFAULT_TIME;
    task->regs.x30 = va;
    task->regs.elr_el1 = va;
    task->regs.spsr_el1 = kernel ? EL1H_M : EL0T_M;
    task->regs.sp_el0 = GRANULE_1GB * 2 + PAGE_SIZE;
    task->regs.sp = (unsigned long)malloc(PAGE_SIZE * 4) + PAGE_SIZE * 4;

    task->mmu_ctx.pgd = (unsigned long *)malloc(PAGE_SIZE);
    task->mmu_ctx.sp_el0_kernel = (VirtualAddr)malloc(PAGE_SIZE) + PAGE_SIZE;

    task->mmu_ctx.pa = VIRT_TO_PHYS(malloc(PAGE_SIZE));
    task->mmu_ctx.va = va;

    task->map_root = malloc(sizeof(TaskMappingNode));
    memset(task->map_root, 0, sizeof(TaskMappingNode));

    for (int i = 0; i < ASID_CHUNKS_NUMBER; i++)
    {
        bool found_asid = false;

        for (int j = 0; j < 256; j++)
        {
            if (!asid_chunks[i][j])
            {
                found_asid = true;
                task->mmu_ctx.asid = j;
                task->mmu_ctx.asid_chunk = i;
                asid_chunks[i][j] = task;
                break;
            }
        }

        if (found_asid)
            break;
    }

    memset(task->mmu_ctx.pgd, 0, PAGE_SIZE);

    char user = kernel ? 0 : MMU_USER;
    char exec = kernel ? 0 : MMU_USER_EXEC;
    char rw = kernel ? MMU_NORW : MMU_RWRW;

    if (data_pa != 0 && va < HIGH_VA)
        MapTaskPage(task, task->mmu_ctx.va, data_pa, PAGE_SIZE, user | exec | rw);
    MapTaskPage(task, task->regs.sp_el0 - PAGE_SIZE, VIRT_TO_PHYS(task->mmu_ctx.sp_el0_kernel - PAGE_SIZE), PAGE_SIZE, user | rw);

    task->regs.x0 = task->regs.x1 = task->regs.x2 = task->regs.x3 = 0;

    if (argc > 0)
    {
        push_args(task, argc, argv);
        task->regs.x0 = argc;
        task->regs.x1 = task->regs.sp_el0;
    }

    if (envargv != 0 && envargc % 2 == 0)
    {
        push_args(task, envargc, envargv);
        task->regs.x2 = envargc;
        task->regs.x3 = task->regs.sp_el0;
    }

    return task;
}