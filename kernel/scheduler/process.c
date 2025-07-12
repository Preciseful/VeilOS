#include <scheduler/process.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/printf.h>

task_t *pcreate(void *pc)
{
    task_t *task = malloc(sizeof(task_t));
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = 0;
    task->regs.sp = (unsigned long)task + PAGE_SIZE;
    task->pgd = (unsigned long *)malloc(PAGE_SIZE);
    memset(task->pgd, 0, PAGE_SIZE);

    mmu_map_page(task->pgd, 0, (unsigned long)pc, MAIR_IDX_NORMAL);
    mmu_map_page(task->pgd, (unsigned long)task, (unsigned long)task, MAIR_IDX_NORMAL);
    mmu_map_page(task->pgd, (unsigned long)&cpu_switch_task, (unsigned long)&cpu_switch_task, MAIR_IDX_NORMAL);

    for (unsigned long addr = GRANULE_1GB * 3; addr < GRANULE_1GB * 4; addr += GRANULE_2MB)
        mmu_map_block(task->pgd, addr, addr, MAIR_IDX_DEVICE);

    add_task(task);
    return task;
}