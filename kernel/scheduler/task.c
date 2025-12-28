#include <scheduler/task.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/scheduler.h>
#include <lib/string.h>
#include <bundles/elf.h>

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
                LOG("Error occured: node is 0.\n");
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

    for (int i = 0; i < task->environc; i++)
        free((void *)PHYS_TO_VIRT(task->environ[i]));
    free(task->environ);

    for (int i = 0; i < task->argc; i++)
        free((void *)PHYS_TO_VIRT(task->argv[i]));
    free(task->argv);

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

void set_args(Task *task, int argc, char **argv, char **environ)
{
    unsigned long environ_len = 0;
    // environ is terminated with zero
    while (environ && environ[environ_len])
        environ_len++;

    task->environc = environ_len;
    task->environ = mapped_malloc(task, (environ_len + 1) * sizeof(char *));
    task->environ[environ_len] = 0;

    for (unsigned long i = 0; i < environ_len; i++)
    {
        unsigned long environ_var_len = strlen(environ[i]) + 1;

        task->environ[i] = (void *)VIRT_TO_PHYS(mapped_malloc(task, environ_var_len));
        memcpy((void *)PHYS_TO_VIRT(task->environ[i]), environ[i], environ_var_len);
    }

    task->argc = argc;
    task->argv = mapped_malloc(task, (argc + 1) * sizeof(char *));
    task->argv[argc] = 0;

    for (unsigned long i = 0; i < argc; i++)
    {
        unsigned long argv_len = strlen(argv[i]) + 1;

        task->argv[i] = (void *)VIRT_TO_PHYS(mapped_malloc(task, argv_len));
        memcpy((void *)PHYS_TO_VIRT(task->argv[i]), argv[i], argv_len);
    }
}

Task *CreateTask(const char *name, bool kernel, VirtualAddr va, PhysicalAddr data_pa, char **environ, char **argv, int argc)
{
    Task *task = malloc(sizeof(Task));

    unsigned long name_len = strlen(name) + 1;
    task->name = malloc(name_len);
    memcpy(task->name, name, name_len);

    task->flags = ACTIVE_TASK;
    task->kernel = kernel;
    task->time = DEFAULT_TIME;
    task->regs.x30 = va;
    task->regs.elr_el1 = va;
    task->regs.spsr_el1 = kernel ? EL1H_M : EL0T_M;
    task->regs.task_sp = GRANULE_1GB * 2 + PAGE_SIZE;
    task->regs.interrupt_sp = (unsigned long)malloc(PAGE_SIZE * 4) + PAGE_SIZE * 4;

    task->mmu_ctx.pgd = (unsigned long *)malloc(PAGE_SIZE);
    task->mmu_ctx.sp_alloc = (unsigned long)malloc(PAGE_SIZE);

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
    MapTaskPage(task, task->regs.task_sp - PAGE_SIZE, VIRT_TO_PHYS(task->mmu_ctx.sp_alloc), PAGE_SIZE, user | rw);

    set_args(task, argc, argv, environ);
    task->regs.x[0] = task->argc;
    task->regs.x[1] = VIRT_TO_PHYS(task->argv);

    return task;
}