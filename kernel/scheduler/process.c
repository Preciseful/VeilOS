#include <scheduler/process.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/printf.h>

#define EL1H_M 0b0101
#define EL0T_M 0b0000

task_t *pcreate(unsigned long pa, unsigned long va, bool kernel)
{
    task_t *task = malloc(sizeof(task_t));
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = va;
    task->regs.spsr = kernel ? EL1H_M : EL0T_M;
    task->regs.sp = (unsigned long)task + PAGE_SIZE;
    task->pgd = (unsigned long *)malloc(PAGE_SIZE);
    memset(task->pgd, 0, PAGE_SIZE);

    mmu_map_page(task->pgd, va, pa, MAIR_IDX_NORMAL, kernel);
    mmu_map_page(task->pgd, (unsigned long)task, (unsigned long)task, MAIR_IDX_NORMAL, kernel);
    mmu_map_page(task->pgd, (unsigned long)&cpu_switch_task, (unsigned long)&cpu_switch_task, MAIR_IDX_NORMAL, true);

    for (unsigned long addr = GRANULE_1GB * 3; addr < GRANULE_1GB * 4; addr += GRANULE_2MB)
        // todo: remove this bc we will just use svc anyways
        mmu_map_block(task->pgd, addr, addr, MAIR_IDX_DEVICE, kernel);

    add_task(task);
    return task;
}