#include <interrupts.h>
#include <Veil.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <drivers/miniuart.h>
#include <mm.h>
#include <lib/base.h>
#include <scheduler.h>
#include <lib/fork.h>

#define IRQ ((struct irq_registers *)(PERIPHERAL_BASE + 0x0000B200))

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

void handle_irq(unsigned long *stack)
{
    unsigned int irq = IRQ->irq0_pending_0;

    while (irq & (SYS_TIMER_IRQ_1 | SYS_TIMER_IRQ_3))
    {
        if (irq & SYS_TIMER_IRQ_1)
        {
            set_stack(stack);
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

unsigned long handle_svc(unsigned long svc, unsigned long *stack)
{
    unsigned long x0 = stack[2], x1 = stack[3], x2 = stack[4], x3 = stack[5], x4 = stack[6], x5 = stack[7];

    switch (svc)
    {
    // test
    case 0:
        INFO("x0: %lu\n", x0);
        INFO("x1: %lu\n", x1);
        INFO("x2: %lu\n", x2);
        INFO("x3: %lu\n", x3);
        INFO("x4: %lu\n", x4);
        INFO("x5: %lu\n", x5);
        return 123;

    case 1:
        return valloc(x0);

    case 2:
        vfree((void *)x0);
        return 0;

    case 3:
        iosend(scheduler_current->io, x0);
        return 0;

    case 4:
        return clone();

    case 5:
        if (scheduler_current->io->status != ACTIVE)
        {
            unsigned long *addon = (unsigned long *)(stack + 34);
            *addon = *addon - 4;
            return x0;
        }
        return scheduler_current->io->read();

    default:
        INFO("got svc huh %lu\n", svc);
        return 0;
    }
}

unsigned long interrupt_message(unsigned long type, unsigned long esr, unsigned long elr, unsigned long *stack, unsigned long svc)
{
    unsigned int ec = esr >> 26;
    unsigned int il = esr >> 31;
    printf_use_framebuffer = false;
    unsigned long *addon = (unsigned long *)(stack + 0);
    unsigned long far = stack[1];
    *addon = 0;

    if (ec == 0b010101)
    {
        unsigned long ret = handle_svc(svc, stack);
        // TODO: remove tasks that abuse svc for far too long because it seems it may kill the timer peripheral and idk how to fix that lol
        *addon = 2;
        printf_use_framebuffer = true;
        return ret;
    }

    WARN("interrupt encountered:\n"
         "-> type %lu\n"
         "-> esr %lu\n"
         "-> elr 0x%lX\n"
         "-> il %u\n"
         "-> far %lu\n",
         type, esr, elr, il, far);

    switch (ec)
    {

    case 0b100100:
        ERROR("-> type of interrupt: data abort from lower EL\n");
        goto leave;

    case 0b111100:
        WARN("-> type of interrupt: brk\n");
        *addon = 1;
        break;
    case 0b100101:
        ERROR("-> type of interrupt: data abort\n");
        goto leave;

    case 0b000000:
        ERROR("-> type of interrupt: unknown reason\n");
        goto leave;

    default:
        ERROR("exception type not implemented\n");
    leave:
        veil();
        break;
    }

    printf_use_framebuffer = true;
    return 0;
}