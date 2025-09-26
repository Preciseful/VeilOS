#include <scheduler/task.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/scheduler.h>

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

Task *CreateTask(const char *name, VirtualAddr va, VirtualAddr code)
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

    task->mmu_ctx.pa = VIRT_TO_PHYS((unsigned long)malloc(PAGE_SIZE));
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

    return task;
}
