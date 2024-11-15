#include <interrupts.h>
#include <Veil.h>
#include <lib/printf.h>
#include <drivers/timer.h>

#define TIMER_HANDLERS_ACCESS

void enable_vc_irq(enum videocore_irqs irq)
{
    IRQ->irq0_disable_0 &= ~irq;
    IRQ->irq0_enable_0 |= irq;
}

void disable_vc_irq(enum videocore_irqs irq)
{
    IRQ->irq0_disable_0 |= irq;
    IRQ->irq0_enable_0 &= ~irq;
}

void handle_irq()
{
    unsigned int irq = IRQ->irq0_pending_0;

    while (irq & (SYS_TIMER_IRQ_1 | SYS_TIMER_IRQ_3))
    {
        if (irq & SYS_TIMER_IRQ_1)
        {
            irq &= ~SYS_TIMER_IRQ_1;
            handle_timer_1();
        }

        if (irq & SYS_TIMER_IRQ_3)
        {
            irq &= ~SYS_TIMER_IRQ_3;
            handle_timer_3();
        }
    }
}

void interrupt_message(unsigned long type, unsigned long esr, unsigned long elr, unsigned int *addon)
{
    unsigned int ec = esr >> 26;
    unsigned int il = esr >> 31;
    printf("interrupt encountered:\n"
           "-> type %lu\n"
           "-> esr %lu\n"
           "-> elr %lu\n"
           "-> il %u\n",
           type, esr, elr, il);

    switch (ec)
    {
    case 0b111100:
        printf("-> type of interrupt: brk\n");
        (*addon) = 1;
        break;

    case 0b000001:
        printf("the voices..");
    case 0b000000:
        printf("-> type of interrupt: unknown reason\n");

    default:
        printf("exception type not implemented\n");
        veil();
        break;
    }
}