#define SYNC_INVALID_EL1t		0
#define IRQ_INVALID_EL1t		1
#define FIQ_INVALID_EL1t		2
#define ERROR_INVALID_EL1t		3

#define SYNC_INVALID_EL1h		4
#define IRQ_INVALID_EL1h		5
#define FIQ_INVALID_EL1h		6
#define ERROR_INVALID_EL1h		7

#define SYNC_INVALID_EL0_64		8
#define IRQ_INVALID_EL0_64		9
#define FIQ_INVALID_EL0_64		10
#define ERROR_INVALID_EL0_64	11

#define SYNC_INVALID_EL0_32		12
#define IRQ_INVALID_EL0_32		13
#define FIQ_INVALID_EL0_32		14
#define ERROR_INVALID_EL0_32	15

//stack frame size
#define S_FRAME_SIZE			272

.macro	kernel_entry, el
	sub	sp, sp, #S_FRAME_SIZE
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]

	.if	\el == 0
		mrs	x21, sp_el0
	.else
		add	x21, sp, #S_FRAME_SIZE
	.endif /* \el == 0 */

	mrs	x22, elr_el1
	mrs	x23, spsr_el1

	stp	x30, x21, [sp, #16 * 15]
	stp	x22, x23, [sp, #16 * 16]
.endm

.macro	kernel_exit, el
	ldp	x22, x23, [sp, #16 * 16]
	ldp	x30, x21, [sp, #16 * 15]

	.if	\el == 0
		msr	sp_el0, x21
	.endif /* \el == 0 */

	msr	elr_el1, x22
	msr	spsr_el1, x23


	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	add	sp, sp, #S_FRAME_SIZE
	eret
.endm

.macro handle_invalid_entry el, type
	kernel_entry \el

	str xzr, [sp, #-16]!
	mrs x0, far_el1
	str x0, [sp, #8]
	mov x4, x8
	mov x0, #\type
    mrs x1, esr_el1
    mrs x2, elr_el1
	mov x3, sp

	bl interrupt_message

	mov x2, x0

	ldr x3, [sp, #0]
	add sp, sp, #16
	cbz x3, 2f
	cmp x3, #1
	bne 1f
	ldp	x22, x23, [sp, #16 * 16]
	add x22, x22, #4
	stp	x22, x23, [sp, #16 * 16]
	b 2f

1:

	ldp	x0, x1, [sp, #16 * 0]
	mov x0, x2
	stp	x0, x1, [sp, #16 * 0]
	b 2f

2:  kernel_exit \el
.endm

.macro	ventry	label
.align	7
	b	\label
.endm

// Exception vectors table
.align	11
.global vectors
vectors:
	ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	ventry	sync_invalid_el1h			// Synchronous EL1h
	ventry	handle_el1_irq				// IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	ventry	sync_invalid_el0_64			// Synchronous 64-bit EL0
	ventry	handle_el0_irq				// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64		 // Error 64-bit EL0

	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32		// Error 32-bit EL0


sync_invalid_el1t:
	handle_invalid_entry 1,  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  1, IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  1, FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  1, ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry 1, SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  1, FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  1, ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  0, SYNC_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  0, FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  0, ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  0, SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  0, IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  0, FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  0, ERROR_INVALID_EL0_32

handle_el1_irq:
	kernel_entry 1
	mov x0, sp
	bl	handle_irq
	kernel_exit 1

handle_el0_irq:
	kernel_entry 0
	mov x0, sp
	bl	handle_irq
	kernel_exit 0

.global err_hang
err_hang: b err_hang

.global interrupt_init_vectors
interrupt_init_vectors:
    adr x0, vectors
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

.global irq_barrier
irq_barrier:
    dsb sy
    ret

.global breakpoint_enable
breakpoint_enable:
	msr daifclr, #0
	ret

.global breakpoint_disable
breakpoint_disable:
	msr daifset, #0
	ret

.global ret_to_user
ret_to_user:
	bl irq_disable
	kernel_exit 0