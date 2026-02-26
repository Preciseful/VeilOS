#pragma once

extern void set_vtable();
extern void irq_enable();
extern void irq_disable();

typedef struct InterruptStack
{
    unsigned long x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27;
    unsigned long sp_el0, sp;
    unsigned long elr_el1, spsr_el1;
    unsigned long x28, x29, x30;
} InterruptStack;