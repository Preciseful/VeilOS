#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <boot/base.h>

#define UART0_DR (PERIPHERAL_BASE + 0x201000)
#define UART0_FR (PERIPHERAL_BASE + 0x201018)
#define UART0_IBRD (PERIPHERAL_BASE + 0x201024)
#define UART0_FBRD (PERIPHERAL_BASE + 0x201028)
#define UART0_LCRH (PERIPHERAL_BASE + 0x20102C)
#define UART0_CR (PERIPHERAL_BASE + 0x201030)
#define UART0_IMSC (PERIPHERAL_BASE + 0x201038)

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
}

void uart_put(char c)
{
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

char uart_recv()
{
    while (mmio_read(UART0_FR) & (1 << 4))
        ;
    return (char)mmio_read(UART0_DR);
}