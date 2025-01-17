#include <interrupts.h>
#include <Veil.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <drivers/miniuart.h>

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

unsigned int interrupt_message(unsigned long type, unsigned long esr, unsigned long elr)
{
    unsigned int ec = esr >> 26;
    unsigned int il = esr >> 31;
    printf_use_framebuffer = false;

    printf("interrupt encountered:\n"
           "-> type %lu\n"
           "-> esr %lu\n"
           "-> elr 0x%lX\n"
           "-> il %u\n",
           type, esr, elr, il);

    switch (ec)
    {
    case 0b010101:
        printf("-> type of interrupt: svc\n");
        printf_use_framebuffer = true;
        return 1;

    case 0b111100:
        printf("-> type of interrupt: brk\n");
        printf_use_framebuffer = true;
        return 1;

    case 0b100101:
        printf("-> type of interrupt: data abort\n");
        goto leave;
    case 0b000000:
        printf("-> type of interrupt: unknown reason\n");
        goto leave;
    default:
        printf("exception type not implemented\n");
    leave:
        veil();
        break;
    }

    return 0;
}