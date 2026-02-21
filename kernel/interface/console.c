#include <interface/console.h>
#include <interface/device/iodevice.h>
#include <lib/printf.h>
#include <drivers/framebuffer.h>
#include <drivers/uart.h>

static bool uartDeviceExists;
static bool fbDeviceExists;
static IODevice uartDevice;
static IODevice fbDevice;

void ConsoleRead(char *buf, unsigned long length)
{
    if (uartDeviceExists)
        uartDevice.read(uartDevice.code, buf, length);
}

char ConsoleRecv()
{
    char buf[2] = "";
    if (uartDeviceExists)
        uartDevice.read(uartDevice.code, buf, 1);

    return buf[0];
}

void ConsoleWrite(const char *buf)
{
    if (uartDeviceExists)
        uartDevice.write(uartDevice.code, buf);

    if (fbDeviceExists)
        fbDevice.write(fbDevice.code, buf);
}

void ConsolePutc(char c)
{
    char buf[2] = "";
    buf[0] = c;

    consoleWrite(buf);
}

void ConsoleInit()
{
    uartDeviceExists = GetIODevice(IO_UART, 0, &uartDevice);
    fbDeviceExists = GetIODevice(IO_FRAMEBUFFER, 0, &fbDevice);

    SetPrintf(ConsolePutc);
}