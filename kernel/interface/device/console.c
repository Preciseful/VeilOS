#include <interface/device/console.h>
#include <interface/device/iodevice.h>
#include <lib/printf.h>
#include <drivers/framebuffer.h>
#include <drivers/uart.h>

static bool uartDeviceExists;
static bool fbDeviceExists;
static IODevice uartDevice;
static IODevice fbDevice;
static IODevice console;

void consoleRead(char *buf, unsigned long length)
{
    if (uartDeviceExists)
        uartDevice.read(buf, length);
}

void consoleWrite(const char *buf)
{
    if (uartDeviceExists)
        uartDevice.write(buf);

    if (fbDeviceExists)
        fbDevice.write(buf);
}

void consolePutc(void *p, char c)
{
    char buf[2] = "";
    buf[0] = c;

    consoleWrite(buf);
}

void ConsoleInit()
{
    console.category = IO_CONSOLE;
    console.read = consoleRead;
    console.write = consoleWrite;
    console.code = 0;
    console.owner = malloc(sizeof(PID));

    AddIODevice(console);

    uartDeviceExists = GetIODevice(IO_UART, 0, &uartDevice);
    fbDeviceExists = GetIODevice(IO_FRAMEBUFFER, 0, &fbDevice);

    SetPrintf(0, consolePutc);
}