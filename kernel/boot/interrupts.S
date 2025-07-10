#include "lib/utils.S"

.macro enter_interrupt el
    mrs x0, sctlr_el1
    bic x0, x0, #0x1
    msr sctlr_el1, x0
    isb

    pushp x0, x1
    pushp x2, x3
    pushp x4, x5
    pushp x6, x7
    pushp x8, x9
    pushp x10, x11
    pushp x12, x13
    pushp x14, x15
    pushp x16, x17
    pushp x18, x19
    pushp x20, x21
    pushp x22, x23
    pushp x24, x25
    pushp x26, x27
    pushp x28, x29
    mrs x28, elr_el1
    pushp x30, x28
.endm

.macro exit_interrupt el
    popp x30, x28
    msr elr_el1, x28
    popp x28, x29
    popp x26, x27
    popp x24, x25
    popp x22, x23
    popp x20, x21
    popp x18, x19
    popp x16, x17
    popp x14, x15
    popp x12, x13
    popp x10, x11
    popp x8, x9
    popp x6, x7
    popp x4, x5
    popp x2, x3
    popp x0, x1

    eret
.endm


invalid_hang:
    b invalid_hang

.macro vinvalid type, el
    enter_interrupt \el
    mov x0, #\type
    mrs x1, esr_el1
    mrs x2, elr_el1
    mov x3, x30

    bl handle_vinvalid

    b invalid_hang
.endm

.align 11
.global el1_vectors
el1_vectors:

// ------------------------------------------------------------
// Current EL with SP0
// ------------------------------------------------------------

    .align 7
    sync_current_el_sp0:
        vinvalid 0, 1

	.align 7
    irq_current_el_sp0:
        vinvalid 1, 1

    .align 7
    fiq_current_el_sp0:
        vinvalid 2, 1

    .align 7
    serror_current_el_sp0:
        vinvalid 3, 1

    // ------------------------------------------------------------
    // Current EL with SPx
    // ------------------------------------------------------------

    .align 7
    sync_current_el_spx:
        vinvalid 4, 1

    .align 7
    irq_current_el_spx:
        b irq_el1x

    .align 7
    fiq_current_el_spx:
        vinvalid 6, 1

    .align 7
    serror_current_el_spx:
        vinvalid 7, 1

    // ------------------------------------------------------------
    // Lower EL using AArch64
    // ------------------------------------------------------------

    .align 7
    sync_lower_el_aarch64:
        vinvalid 8, 0

    .align 7
    irq_lower_el_aarch64:
        b irq_el0x

    .align 7
    fiq_lower_el_aarch64:
        vinvalid 10, 0

    .align 7
    serror_lower_el_aarch64:
        vinvalid 11, 0

    // ------------------------------------------------------------
    // Lower EL using AArch32
    // ------------------------------------------------------------

    .align 7
    sync_lower_el_aarch32:
        vinvalid 12, 0

    .align 7
    irq_lower_el_aarch32:
        vinvalid 13, 0

    .align 7
    fiq_lower_el_aarch32:
        vinvalid 14, 0

    .align 7
    serror_lower_el_aarch32:
        vinvalid 15, 0


irq_el1x:
    enter_interrupt 1
    mov x0, sp
    bl handle_irq
    exit_interrupt 1

irq_el0x:
    enter_interrupt 0
    mov x0, sp
    bl handle_irq
    exit_interrupt 0

.global set_vtable
set_vtable:
    adr x0, el1_vectors
    msr vbar_el1, x0
    ret

.global irq_enable
irq_enable:
    msr daifclr, #2
    ret

.global irq_disable
irq_disable:
    msr daifset, #2
    ret

.global ret_to_user
ret_to_user:
	bl irq_disable
	exit_interrupt 0