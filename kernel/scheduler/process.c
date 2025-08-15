#include <scheduler/process.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/printf.h>
#include <syscall/syscall.h>

#define EL1H_M 0b0101
#define EL0T_M 0b0000

extern unsigned char el1_vectors[];

task_t *pcreate(unsigned long pa, unsigned long va)
{
    task_t *task = malloc(sizeof(task_t));
    task->flags = ACTIVE_TASK;
    task->time = DEFAULT_TIME;
    task->regs.x30 = va;
    task->regs.elr = va;
    task->regs.spsr = EL0T_M;
    task->regs.sp = va + PAGE_SIZE;
    task->regs.sp_el1 = (unsigned long)malloc(PAGE_SIZE_BYTES) + PAGE_SIZE;
    task->pgd = (unsigned long *)malloc(PAGE_SIZE);

    memset(task->pgd, 0, PAGE_SIZE);
    mmu_map_page(task->pgd, va, pa, MAIR_IDX_NORMAL, false);
    // map as such for lower half
    // (temporary fix, sys_printf should be implemented into the executable itself)
    mmu_map_page(task->pgd, VIRT_TO_PHYS((unsigned long)&sys_printf), VIRT_TO_PHYS((unsigned long)&sys_printf), MAIR_IDX_NORMAL, false);

    add_task(task);
    return task;
}
