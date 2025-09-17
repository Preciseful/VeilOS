
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <lib/printf.h>
#include <interface/portal.h>

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
    UartPut(c);
}

void UartInit()
{
    SetAlt0(14);
    SetAlt0(15);

    PullGPIO(14, GPIO_PULL_NONE);
    PullGPIO(15, GPIO_PULL_NONE);

    WriteToMMIO(UART0_CR, 0);
    WriteToMMIO(UART0_IBRD, 26);
    WriteToMMIO(UART0_FBRD, 3);
    WriteToMMIO(UART0_LCRH, 3 << 5);
    WriteToMMIO(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    WriteToMMIO(UART0_IMSC, 1 << 4);

    PrintfInit(0, putc);
}

void UartPut(char c)
{
    if (c == '\n')
        UartPut('\r');
    while (ReadMMIO(UART0_FR) & (1 << 5))
        ;
    WriteToMMIO(UART0_DR, c);
}

void UartPuts(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
            UartPut('\r');
        UartPut(*str++);
    }
}

char UartCharacter()
{
    return (char)ReadMMIO(UART0_DR);
}

char UartRecv()
{
    while (ReadMMIO(UART0_FR) & (1 << 4))
        ;
    return (char)ReadMMIO(UART0_DR);
}

PORTAL_READ_FUNCTION(UartPortalRead)
{
    for (unsigned long i = 0; i < length; i++)
    {
        buf[i] = (unsigned char)UartRecv();
        if (buf[i] == '\r')
            buf[i] = '\n';
    }

    return length;
}

PORTAL_WRITE_FUNCTION(UartPortalWrite)
{
    for (unsigned long i = 0; i < length; i++)
        UartPut((char)buf[i]);

    return length;
}