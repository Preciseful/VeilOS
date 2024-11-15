#ifndef IRQ_H
#define IRQ_H

#include <lib/base.h>

#define IRQ ((struct irq_registers *)(PERIPHERAL_BASE + 0x0000B200))

struct irq_registers
{
    volatile unsigned int irq0_pending_0;
    volatile unsigned int irq0_pending_1;
    volatile unsigned int irq0_pending_2;
    volatile unsigned int reserved_0;
    volatile unsigned int irq0_enable_0;
    volatile unsigned int irq0_enable_1;
    volatile unsigned int irq0_enable_2;
    volatile unsigned int reserved_1;
    volatile unsigned int irq0_disable_0;
    volatile unsigned int irq0_disable_1;
    volatile unsigned int irq0_disable_2;
};

enum videocore_irqs
{
    SYS_TIMER_IRQ_0 = 1,
    SYS_TIMER_IRQ_1 = 2,
    SYS_TIMER_IRQ_2 = 4,
    SYS_TIMER_IRQ_3 = 8,
    VIDEOCORE_AUX_IRQ = (1 << 29)
};

extern void interrupt_init_vectors();
extern void irq_enable();
extern void irq_barrier();
extern void irq_disable();
extern void breakpoint_enable();

void enable_vc_irq(enum videocore_irqs irq);
void disable_vc_irq(enum videocore_irqs irq);
void handle_irq();

#endif