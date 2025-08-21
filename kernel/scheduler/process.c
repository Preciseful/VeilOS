#include <scheduler/process.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <syscall/syscall.h>

#define EL1H_M 0b0101
#define EL0T_M 0b0000
#define ASID_CHUNKS_NUMBER 255

extern unsigned char el1_vectors[];

Task *asid_chunks[ASID_CHUNKS_NUMBER][255] = {};

bool TaskContainsVA(Task *task, VirtualAddr va)
{
    for (unsigned long i = 0; i < task->mappings_length; i++)
    {
        if (task->mappings[i].va == va)
            return true;
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

    TaskMapping map;
    map.code = code;
    map.va = va;
    map.pa = pa;

    TaskMapping *old_mappings = task->mappings;
    task->mappings = malloc(sizeof(TaskMapping) * (task->mappings_length + 1));
    memcpy(task->mappings, old_mappings, sizeof(TaskMapping) * task->mappings_length);

    task->mappings[task->mappings_length] = map;
    task->mappings_length++;

    if (old_mappings != 0)
        free(old_mappings);
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

Task *PCreate(const char *name, VirtualAddr va, VirtualAddr code)
{
    Task *task = malloc(sizeof(Task));
    task->name = name;
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = va;
    task->regs.elr = va;
    task->regs.spsr = EL0T_M;
    task->regs.sp = GRANULE_1GB * 2 + PAGE_SIZE;
    task->regs.sp_el1 = (unsigned long)malloc(PAGE_SIZE) + PAGE_SIZE;

    task->mmu_ctx.pgd = (unsigned long *)malloc(PAGE_SIZE);
    task->mmu_ctx.sp_alloc = (unsigned long)malloc(PAGE_SIZE);
    task->mappings = 0;
    task->mappings_length = 0;
    task->mmu_ctx.pa = VIRT_TO_PHYS((unsigned long)malloc(PAGE_SIZE));
    task->mmu_ctx.va = va;

    for (int i = 0; i < ASID_CHUNKS_NUMBER; i++)
    {
        bool found_asid = false;

        for (int j = 0; j < 255; j++)
        {
            if (!asid_chunks[i][j])
            {
                found_asid = true;
                task->mmu_ctx.asid = j;
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
    MapTaskPage(task, task->regs.sp - PAGE_SIZE, MMU_USER | MMU_RWRW, task->mmu_ctx.sp_alloc, 1);

    AddTask(task);
    return task;
}
