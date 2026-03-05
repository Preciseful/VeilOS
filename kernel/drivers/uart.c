
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <lib/printf.h>
#include <interface/iodevice.h>
#include <scheduler/scheduler.h>

#define UART0_BASE (PERIPHERAL_BASE + 0x201000)
#define UART0_DR (UART0_BASE + 0x00)
#define UART0_FR (UART0_BASE + 0x18)
#define UART0_IBRD (UART0_BASE + 0x24)
#define UART0_FBRD (UART0_BASE + 0x28)
#define UART0_LCRH (UART0_BASE + 0x2C)
#define UART0_CR (UART0_BASE + 0x30)
#define UART0_IMSC (UART0_BASE + 0x38)

static IODevice uartDevice;
char read_buf[1024];
unsigned long received;
unsigned long read;

void uartPut(char c)
{
    if (c == '\n')
        uartPut('\r');
    while (ReadMMIO(UART0_FR) & (1 << 5))
        ;
    WriteToMMIO(UART0_DR, c);
}

char uartCharacter()
{
    return (char)ReadMMIO(UART0_DR);
}

char uartRecv()
{
    while (received == read)
        ;

    char c = read_buf[read];
    read = (read + 1) % 512;
    return c;
}

unsigned long uartRead(TokenID token, char *buf, unsigned long length)
{
    for (unsigned long i = 0; i < length; i++)
    {
        buf[i] = uartRecv();
        if (buf[i] == '\r')
            buf[i] = '\n';
    }

    return length;
}

unsigned long uartWrite(TokenID token, const char *str)
{
    const char *s_orig = str;

    while (*str)
    {
        if (*str == '\n')
            uartPut('\r');
        uartPut(*str++);
    }

    return str - s_orig;
}

void UartInit()
{
    received = 0;
    read = 0;

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

    SetPrintf(uartPut);

    uartDevice.category = IO_UART;
    uartDevice.read = uartRead;
    uartDevice.write = uartWrite;
    uartDevice.request = 0;
    uartDevice.code = 0;
    uartDevice.flags = 0;
    uartDevice.tokens_length = 1;
    uartDevice.tokens = malloc(sizeof(IODeviceToken) * 1);

    AddIODevice(uartDevice);
}

void UartReceived()
{
    read_buf[received] = uartCharacter();
    received = (received + 1) % 512;
}