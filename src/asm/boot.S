#include "lib/sysregs.h"
#include "lib/base.h"
#include "mm.h"

.section ".text.boot"

.global _start
_start:
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, set_el1
    b loop
    
loop: 
    wfe
    b loop

set_el1:
    mov     x0, #0x33ff
    msr     cptr_el3, x0 	 // Disable coprocessor traps to EL3
    mov     x0, #3 << 20
    msr     cpacr_el1, x0	 // Enable FP/SIMD at EL1

    ldr     x0, =SCTLR_VALUE_MMU_DISABLED
    msr	    sctlr_el1, x0		

    ldr     x0, =HCR_VALUE
    msr     hcr_el2, x0

    ldr     x0, =SCR_VALUE
    msr     scr_el3, x0

    ldr     x0, =SPSR_VALUE
    msr     spsr_el3, x0
    
    adr     x0, main_core		
    msr     elr_el3, x0

    eret	

main_core:
    ldr x1, =__bss_start
    ldr x2, =__bss_size
1:  cbz x2, 2f
    str xzr, [x1], #8
    sub x2, x2, #1
    cbnz x2, 1b
    mov sp, #(2 * SECTION_SIZE)
2:  bl unveil
    b loop
    