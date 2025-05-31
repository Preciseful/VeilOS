#include <drivers/uart.h>
#include <lib/printf.h>

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
}