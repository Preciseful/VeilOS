#include "boot/mmu.h"
#include "boot/sysregs.h"

.section ".text.boot"

.global _start
_start:
    mrs x1, mpidr_el1
    and x1, x1, #0xFF
    cbz x1, drop_el1
    b loop

loop:
    b loop

drop_el1:
    ldr	x0, =SCTLR_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0

	ldr	x0, =HCR_VALUE
	msr	hcr_el2, x0

	ldr	x0, =SCR_VALUE
	msr	scr_el3, x0

	ldr	x0, =SPSR_VALUE
	msr	spsr_el3, x0

	adr	x0, master_core
	msr	elr_el3, x0

	eret

master_core:
    adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl memzero

    // bl mmu_enable

    mov	sp, #LOW_MEMORY
    bl kmain
    b loop