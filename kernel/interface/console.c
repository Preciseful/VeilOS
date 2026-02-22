#include <interface/console.h>
#include <interface/iodevice.h>
#include <lib/printf.h>
#include <drivers/framebuffer.h>
#include <drivers/uart.h>

static int uartToken;
static int fbToken;

void ConsoleRead(char *buf, unsigned long length)
{
    if (uartToken != -1)
        ReadIODevice(uartToken, IO_UART, 0, buf, length);
}

char ConsoleRecv()
{
    char buf;
    if (uartToken != -1)
        ReadIODevice(uartToken, IO_UART, 0, &buf, 1);

    return buf;
}

void ConsoleWrite(const char *buf)
{
    if (uartToken != -1)
        WriteIODevice(uartToken, IO_UART, 0, buf);

    if (fbToken != -1)
        WriteIODevice(fbToken, IO_FRAMEBUFFER, 0, buf);
}

void ConsolePutc(char c)
{
    char buf[2] = "";
    buf[0] = c;

    ConsoleWrite(buf);
}

void ConsoleInit()
{
    uartToken = OwnIODevice(IO_UART, 0, IO_READ | IO_WRITE);
    fbToken = OwnIODevice(IO_FRAMEBUFFER, 0, IO_WRITE | IO_REQUEST);

    SetPrintf(ConsolePutc);
}

void ConsoleDrop()
{
    FreeIODevice(uartToken, IO_UART, 0);
    FreeIODevice(fbToken, IO_FRAMEBUFFER, 0);
}