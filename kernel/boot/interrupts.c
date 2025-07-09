#include <boot/interrupts.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/gpio.h>
#include <drivers/gic.h>
#include <drivers/timer.h>
#include <drivers/uart.h>
#include <scheduler/scheduler.h>

void handle_vinvalid(unsigned long type, unsigned long esr, unsigned long elr, unsigned long far)
{
    printf("\ninterrupt encountered:"
           "\n\ttype: %lu"
           "\n\tesr: %lu"
           "\n\telr: %lx (%lx)"
           "\n\tfar: %lx (%lx)",
           type, esr, elr, elr - 0x80000, far, far - 0x80000);
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