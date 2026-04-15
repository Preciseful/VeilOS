/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "memory/mmu.h"
#include "memory/memory.h"
#include "boot/sysregs.h"
#include "drivers/gic.h"

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
    ldr	x0, =SCTLR_EL1_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0

	ldr	x0, =HCR_EL2_VALUE
	msr	hcr_el2, x0

	ldr	x0, =SCR_EL3_VALUE
	msr	scr_el3, x0

	ldr	x0, =SPSR_EL3_VALUE
	msr	spsr_el3, x0

    mrs x0, CPACR_EL1
    orr x0, x0, #(3 << 20)
    msr CPACR_EL1, x0

	adr	x0, master_core
	msr	elr_el3, x0

	eret

memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero
    ret

master_core:
    mov	sp, #LOW_MEMORY

    // clear out bss
    adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl memzero

    // go to boot for MMU
    bl kboot
    b loop