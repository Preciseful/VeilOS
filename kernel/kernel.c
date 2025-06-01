#include <drivers/uart.h>
#include <drivers/gic.h>
#include <boot/interrupts.h>
#include <lib/printf.h>
#include <drivers/timer.h>

void putc(void *p, char c)
{
    uart_put(c);
}

void kmain()
{
    uart_init();
    init_printf(0, putc);

    uart_puts("health 2021\n");

    int x = 2021;
    printf("how to move on from %d;", x);

    set_vtable();
    timer_init();
    gic_allow(30, 0);
    gic_allow(153, 0);
    irq_enable();

    while (1)
    {
    }
}