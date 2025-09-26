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
    ListObject *obj = task->mappings.first;
    while (obj)
    {
        if (GET_VALUE(obj, TaskMapping)->va == va)
            return true;
        obj = obj->next;
    }

    return false;
}

void MapTaskPage(Task *task, VirtualAddr va, enum MMU_Flags flags, VirtualAddr code, unsigned long code_len)
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

void *mapped_malloc(Task *task, unsigned int size)
{
    void *data = malloc(size);
    MapTaskPage(task, VIRT_TO_PHYS(data), MMU_USER | MMU_RWRW, (VirtualAddr)data, size);
    return data;
}

void set_args(Task *task, int argc, char **argv, char **environ)
{
    unsigned long environ_len = 0;
    // environ is terminated with zero
    while (environ && environ[environ_len] != 0)
        environ_len++;

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
    task->name = name;
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
        MapTaskPage(task, task->mmu_ctx.va, MMU_USER | MMU_USER_EXEC | MMU_RWRW, code, PAGE_SIZE);
    MapTaskPage(task, task->regs.sp_el0 - PAGE_SIZE, MMU_USER | MMU_RWRW, task->mmu_ctx.sp_alloc, 1);

    set_args(task, argc, argv, environ);
    task->regs.x[0] = task->argc;
    task->regs.x[1] = VIRT_TO_PHYS(task->argv);

    return task;
}
