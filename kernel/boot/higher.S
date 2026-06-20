#include "memory/mmu.h"
#include "memory/allocator.h"

.global jump_high
jump_high:
    mov x3, x2
    mov x2, x1
    mov x1, x0

    ldr x0, =MAIR_VALUE
    msr mair_el1, x0

    ldr x0, =TCR_VALUE
    msr tcr_el1, x0

    dsb ish
    isb

    msr ttbr0_el1, x1
    msr ttbr1_el1, x2

    adr x30, finish_higher

    mrs x0, sctlr_el1
    orr x0, x0, #0x1
    bic x0, x0, #(1 << 19)
    msr sctlr_el1, x0
    isb

    ldr x1, =HIGH_VA

    // set all these variables to higher half
    mrs x0, vbar_el1
    add x0, x0, x1
    msr vbar_el1, x0

    mov sp, #LOW_MEMORY
    add sp, sp, x1
    // x30 = finish_higher address
    // go to higher half
    add x30, x30, x1

    mov x0, x3
    br x30