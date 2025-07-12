#include "memory/mmu.h"

.global mmu_init_regs
mmu_init_regs:
    mov x1, x0

    ldr x0, =MAIR_VALUE
    msr mair_el1, x0

    ldr x0, =TCR_VALUE
    msr tcr_el1, x0

    dsb ish
    isb

    msr ttbr0_el1, x1

    mrs x0, sctlr_el1
    orr x0, x0, #0x1
    bic x0, x0, #(1 << 19)
    msr sctlr_el1, x0
    isb

    ret