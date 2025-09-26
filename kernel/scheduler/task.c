#include <scheduler/task.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/scheduler.h>
#include <lib/string.h>

#define EL1H_M 0b0101
#define EL0T_M 0b0000
#define ASID_CHUNKS_NUMBER 256

extern unsigned char el1_vectors[];

Task *asid_chunks[ASID_CHUNKS_NUMBER][256] = {};

bool TaskContainsVA(Task *task, VirtualAddr va)
{
    for (ListObject *obj = task->mappings.first; obj; obj = obj->next)
    {
        if (GET_VALUE(obj, TaskMapping)->va == va)
            return true;
    }

    return false;
}

void MapTaskPage(Task *task, VirtualAddr va, enum MMU_Flags flags, VirtualAddr code, unsigned long code_len,
                 enum Task_Mapping_Properties properties_to_free)
{
    PhysicalAddr pa = VIRT_TO_PHYS(code);
    unsigned long num_pages = (code_len + PAGE_SIZE - 1) / PAGE_SIZE;

    for (unsigned long i = 0; i < num_pages; i++)
    {
        unsigned long offset = i * PAGE_SIZE;
        MapTablePage(task->mmu_ctx.pgd, va + offset, pa + offset, MAIR_IDX_NORMAL, flags);
    }

    TaskMapping *map = malloc(sizeof(TaskMapping));
    map->code = code;
    map->va = va;
    map->pa = pa;
    map->to_free = properties_to_free;

    AddToList(&task->mappings, map);
}

void UnmapTaskPage(Task *task, VirtualAddr va, unsigned long length)
{
    if (!TaskContainsVA(task, va))
        return;

    unsigned long num_pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned long i = 0; i < num_pages; i++)
    {
        unsigned long offset = i * PAGE_SIZE;
        UnmapTablePage(task->mmu_ctx.pgd, va + offset);
    }
}

void KillTask(Task *task)
{
    for (ListObject *obj = task->mappings.first; obj; obj = obj->next)
    {
        TaskMapping *mapping = GET_VALUE(obj, TaskMapping);
        if (mapping->to_free & MAP_PROPERTY_CODE)
            free((void *)mapping->code);
        if (mapping->to_free & MAP_PROPERTY_PA)
            free((void *)mapping->pa);
        if (mapping->to_free & MAP_PROPERTY_VA)
            free((void *)mapping->va);
    }

    FreeList(&task->mappings, true);

    for (int i = 0; i < task->environc; i++)
        free(task->environ[i]);
    free(task->environ);

    for (int i = 0; i < task->argc; i++)
        free(task->argv[i]);
    free(task->argv);

    free(task->name);
    FreeTable(task->mmu_ctx.pgd, 0);
    free(task->mmu_ctx.pa);
    free(task);
}

void *mapped_malloc(Task *task, unsigned int size)
{
    void *data = malloc(size);
    MapTaskPage(task, VIRT_TO_PHYS(data), MMU_USER | MMU_RWRW, (VirtualAddr)data, size, MAP_PROPERTY_CODE);
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

Task *CreateTask(const char *name, VirtualAddr va, VirtualAddr code, char **environ, char **argv, int argc)
{
    Task *task = malloc(sizeof(Task));

    unsigned long name_len = strlen(name) + 1;
    task->name = malloc(name_len);
    memcpy(task->name, name, name_len);

    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = va;
    task->regs.elr_el1 = va;
    task->regs.spsr_el1 = EL0T_M;
    task->regs.sp_el0 = GRANULE_1GB * 2 + PAGE_SIZE;
    task->regs.sp_el1 = (unsigned long)malloc(PAGE_SIZE * 4) + PAGE_SIZE * 4;

    task->mmu_ctx.pgd = (unsigned long *)malloc(PAGE_SIZE);
    task->mmu_ctx.sp_alloc = (unsigned long)malloc(PAGE_SIZE);
    task->mappings = CreateList(LIST_LINKED);

    task->mmu_ctx.pa = VIRT_TO_PHYS(malloc(PAGE_SIZE));
    task->mmu_ctx.va = va;

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

    if (code != 0)
        MapTaskPage(task, task->mmu_ctx.va, MMU_USER | MMU_USER_EXEC | MMU_RWRW, code, PAGE_SIZE, MAP_PROPERTY_CODE);
    MapTaskPage(task, task->regs.sp_el0 - PAGE_SIZE, MMU_USER | MMU_RWRW, task->mmu_ctx.sp_alloc, 1, MAP_PROPERTY_CODE);

    set_args(task, argc, argv, environ);
    task->regs.x[0] = task->argc;
    task->regs.x[1] = VIRT_TO_PHYS(task->argv);

    return task;
}
