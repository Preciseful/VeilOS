#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <lib/printf.h>

#define UART0_BASE (PERIPHERAL_BASE + 0x201000)
#define UART0_DR (UART0_BASE + 0x00)
#define UART0_FR (UART0_BASE + 0x18)
#define UART0_IBRD (UART0_BASE + 0x24)
#define UART0_FBRD (UART0_BASE + 0x28)
#define UART0_LCRH (UART0_BASE + 0x2C)
#define UART0_CR (UART0_BASE + 0x30)
#define UART0_IMSC (UART0_BASE + 0x38)

void putc(void *p, char c)
{
    uart_put(c);
}

void uart_init()
{
    gpio_setAlt0(14);
    gpio_setAlt0(15);

    gpio_pull(14, Pull_None);
    gpio_pull(15, Pull_None);

    mmio_write(UART0_CR, 0);
    mmio_write(UART0_IBRD, 26);
    mmio_write(UART0_FBRD, 3);
    mmio_write(UART0_LCRH, 3 << 5);
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    mmio_write(UART0_IMSC, 1 << 4);

    init_printf(0, putc);
}

void uart_put(char c)
{
    if (c == '\n')
        uart_put('\r');
    while (mmio_read(UART0_FR) & (1 << 5))
        ;
    mmio_write(UART0_DR, c);
}

void uart_puts(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
            uart_put('\r');
        uart_put(*str++);
    }
}

char uart_character()
{
    return (char)mmio_read(UART0_DR);
}

char uart_recv()
{
    while (mmio_read(UART0_FR) & (1 << 4))
        ;
    return (char)mmio_read(UART0_DR);
}