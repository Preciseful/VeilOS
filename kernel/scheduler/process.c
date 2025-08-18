#include <scheduler/process.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <syscall/syscall.h>

#define EL1H_M 0b0101
#define EL0T_M 0b0000

extern unsigned char el1_vectors[];

bool task_contains_va(task_t *task, unsigned long va)
{
    for (unsigned long i = 0; i < task->mappings_length; i++)
    {
        if (task->mappings[i].va == va)
            return true;
    }

    return false;
}

void map_task_page(task_t *task, unsigned long va, enum MMU_Flags flags, void *code, unsigned long code_len)
{
    unsigned long pa = VIRT_TO_PHYS((unsigned long)code);
    unsigned long num_pages = (code_len + PAGE_SIZE - 1) / PAGE_SIZE;

    for (unsigned long i = 0; i < num_pages; i++)
    {
        unsigned long offset = i * PAGE_SIZE;
        mmu_map_page(task->pgd, va + offset, pa + offset, MAIR_IDX_NORMAL, flags);
    }

    task_mapping_t map;
    map.code = code;
    map.va = va;
    map.pa = pa;

    task_mapping_t *old_mappings = task->mappings;
    task->mappings = malloc(sizeof(task_mapping_t) * (task->mappings_length + 1));
    memcpy(task->mappings, old_mappings, sizeof(task_mapping_t) * task->mappings_length);

    task->mappings[task->mappings_length] = map;
    task->mappings_length++;

    if (old_mappings != 0)
        free(old_mappings);
}

void unmap_task_page(task_t *task, unsigned long va, unsigned long length)
{
    if (!task_contains_va(task, va))
        return;

    unsigned long num_pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned long i = 0; i < num_pages; i++)
    {
        unsigned long offset = i * PAGE_SIZE;
        mmu_unmap_page(task->pgd, va + offset);
    }
}

task_t *pcreate(const char *name, unsigned long va, void *code)
{
    task_t *task = malloc(sizeof(task_t));
    task->name = name;
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->va = va;
    task->regs.x30 = va;
    task->regs.elr = va;
    task->regs.spsr = EL0T_M;
    task->regs.sp = GRANULE_1GB * 2 + PAGE_SIZE;
    task->regs.sp_el1 = (unsigned long)malloc(PAGE_SIZE) + PAGE_SIZE;
    task->pgd = (unsigned long *)malloc(PAGE_SIZE);

    task->phys_sp = (unsigned long)malloc(PAGE_SIZE);
    task->pa = VIRT_TO_PHYS((unsigned long)malloc(PAGE_SIZE));
    task->mappings = 0;
    task->mappings_length = 0;

    memset(task->pgd, 0, PAGE_SIZE);

    if (code != 0)
        map_task_page(task, task->va, MMU_USER_EXEC | MMU_RWRW, code, PAGE_SIZE);
    map_task_page(task, task->regs.sp - PAGE_SIZE, MMU_RWRW, (void *)task->phys_sp, 1);

    add_task(task);
    return task;
}
