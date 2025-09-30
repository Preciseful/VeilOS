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

.global cpu_switch_task
cpu_switch_task:
	// x2 is x0 here
	ldp x2, x1, [x0], #16
	push x2

	ldp x2, x3, [x0], #16
	// temp pair
	pushp x2, x3

	ldp x4, x5, [x0], #16
	ldp x6, x7, [x0], #16
	ldp x8, x9, [x0], #16
	ldp x10, x11, [x0], #16
	ldp x12, x13, [x0], #16
	ldp x14, x15, [x0], #16
	ldp x16, x17, [x0], #16
	ldp x18, x19, [x0], #16
	ldp	x20, x21, [x0], #16
	ldp	x22, x23, [x0], #16
	ldp	x24, x25, [x0], #16
	ldp	x26, x27, [x0], #16

	ldp x2, x3, [x0], #16
	msr sp_el0, x2
	mov x27, x3

	ldp x2, x3, [x0], #16
	msr elr_el1, x2
	msr spsr_el1, x3

	ldp x28, x29, [x0], #16
	ldr x30, [x0], #16

	// bring back x0, x2, x3
	popp x2, x3
	pop x0

	mov sp, x27
	msr spsel, #0
    eret
