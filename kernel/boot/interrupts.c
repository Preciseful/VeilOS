#include <boot/interrupts.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/gpio.h>
#include <drivers/gic.h>
#include <drivers/timer.h>
#include <drivers/uart.h>
#include <scheduler/scheduler.h>

void handle_svc(unsigned long *sp)
{
    unsigned long code = sp[8];
    switch (code)
    {
    case 0:
        printf("%s", sp[0]);
        break;

    default:
        printf("SVC code #%lu does not exist.\n", code);
        break;
    }
}

unsigned long handle_vinvalid(unsigned long type, unsigned long esr, unsigned long elr, unsigned long far, unsigned long *sp)
{
    unsigned int ec = esr >> 26;

    if (ec == 0b010101)
    {
        handle_svc(sp);
        return 1;
    }

    printf("\ninterrupt encountered:"
           "\n\ttype: %lu"
           "\n\tesr: %lu"
           "\n\telr: %lx (%lx)"
           "\n\tfar: %lx (%lx)",
           type, esr, elr, elr - 0x80000, far, far - 0x80000);
    return 0;
}

void handle_irq(unsigned long *stack)
{
    unsigned int iar = mmio_read(GICC_IAR);
    unsigned int id = iar & 0x2FF;

    switch (id)
    {
    // generic timer
    case 30:
        printf(".");
        refresh_cntp_tval(SYS_FREQ);
        mmio_write(GICC_EOIR, iar);
        scheduler_tick(stack);
        break;

    // uart0
    case 153:
        printf("got: %c\n", uart_character());
        mmio_write(GICC_EOIR, iar);
        break;

    default:
        printf("Unknown IRQ %u\n", id);
        mmio_write(GICC_EOIR, iar);
        break;
    }
}