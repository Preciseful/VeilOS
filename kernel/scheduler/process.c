#include <scheduler/process.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/printf.h>
#include <syscall/syscall.h>

#define EL1H_M 0b0101
#define EL0T_M 0b0000

extern unsigned long *el1_vectors;

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
    mmu_map_page(task->pgd, (unsigned long)el1_vectors, (unsigned long)el1_vectors, MAIR_IDX_NORMAL, kernel);
    mmu_map_page(task->pgd, (unsigned long)&sys_printf, (unsigned long)&sys_printf, MAIR_IDX_NORMAL, kernel);

    add_task(task);
    return task;
}