#include <boot/interrupts.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/gpio.h>
#include <drivers/gic.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

void handle_vinvalid(unsigned long type, unsigned long esr, unsigned long elr)
{
    printf("\ninterrupt encountered:"
           "\n\ttype: %lu"
           "\n\tesr: %lu"
           "\n\telr: %lx",
           type, esr, elr);
}

void handle_irq()
{
    unsigned int iar = mmio_read(GICC_IAR);
    unsigned int id = iar & 0x2FF;

    switch (id)
    {
    // generic timer
    case 30:
        printf(".");
        refresh_cntp_tval(SYS_FREQ);
        break;

    // uart0
    case 153:
        printf("got: %c\n", uart_character());
        break;

    default:
        printf("Unknown IRQ %u\n", id);
        break;
    }

    mmio_write(GICC_EOIR, iar);
}