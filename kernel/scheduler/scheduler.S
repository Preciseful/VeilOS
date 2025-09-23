#include "lib/utils.S"

.global set_task_ttbr
set_task_ttbr:
	msr ttbr0_el1, x0
	cmp x1, #0
	beq 1f
    tlbi vmalle1is
1:	dsb ish
	isb
	ret

// x3 = sp_el0
// x4 = sp
// x5 = spsr_el1
// x6 = elr_el1
.global cpu_switch_task
cpu_switch_task:
	// check stack and if interrupt should be restored
	cmp x2, #0
	beq 1f

	ldp	x6, x29, [x2, #16 * 14]
	ldp x30, x28, [x2, #16 * 15]

1:	mrs x3, sp_el0
	mov x4, sp
	mrs x5, spsr_el1

	mov x8, x0

    stp x19, x20, [x8], #16
    stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x30, [x8], #16
	stp	x3, x4, [x8], #16
	stp x5, x6, [x8]

	mov x8, x1

	ldp	x19, x20, [x8], #16
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x30, [x8], #16
	ldp	x3, x4, [x8], #16
	ldp x5, x6, [x8]

	msr sp_el0, x3
	mov sp, x4
	msr spsr_el1, x5
	msr elr_el1, x6

    eret
