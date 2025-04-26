#include <interrupts.h>
#include <Veil.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <drivers/miniuart.h>
#include <mm.h>
#include <lib/base.h>
#include <scheduler.h>

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
        printf("x0: %lu\n", x0);
        printf("x1: %lu\n", x1);
        printf("x2: %lu\n", x2);
        printf("x3: %lu\n", x3);
        printf("x4: %lu\n", x4);
        printf("x5: %lu\n", x5);
        return 123;

    case 1:
        return valloc(x0);

    case 2:
        vfree((void *)x0);
        return 0;

    case 3:
        printf("%s", x0);
        return 0;

    default:
        printf("got svc huh %lu\n", svc);
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
        *addon = 2;
        printf_use_framebuffer = true;
        return ret;
    }

    printf("interrupt encountered:\n"
           "-> type %lu\n"
           "-> esr %lu\n"
           "-> elr 0x%lX\n"
           "-> il %u\n"
           "-> far %lu\n",
           type, esr, elr, il, far);

    switch (ec)
    {

    case 0b100100:
        printf("-> type of interrupt: data abort from lower EL\n");
        goto leave;

    case 0b111100:
        printf("-> type of interrupt: brk\n");
        *addon = 1;
        break;
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

    printf_use_framebuffer = true;
    return 0;
}