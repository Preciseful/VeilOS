#include <drivers/framebuffer.h>
#include <drivers/mailbox.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <interface/device/iodevice.h>
#include <scheduler/scheduler.h>
#include <lib/font.h>
#include <drivers/uart.h>

static IODevice fbDevice;

unsigned char *framebuffer;
unsigned int width, height;
unsigned int pitch;
unsigned int isrgb;
unsigned char colors[3] = {0, 0, 0};

void drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
    if (framebuffer == 0)
        return;

    int offs = (y * pitch) + (x * 4);
    unsigned char *pixels = (unsigned char *)framebuffer + offs;
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
    pixels[3] = 0xFF;
}

void drawChar(unsigned char ch, int x, int y, unsigned char r, unsigned char g, unsigned char b, bool overlay)
{
    if (ch >= FONT_NUMGLYPHS)
        ch = 0;

    for (int i = 0; i < FONT_HEIGHT; i++)
    {
        unsigned char row = ACCESS_FONT(ch, i);

        for (int j = 0; j < FONT_WIDTH; j++)
        {
            if (row & (1 << (7 - j)))
                drawPixel(x + j, y + i, r, g, b);
            else if (!overlay)
                drawPixel(x + j, y + i, 0, 0, 0);
        }
    }
}

void drawString(const char *s, unsigned char r, unsigned char g, unsigned char b, bool overlay)
{
    while (*s)
    {
        if (*s == '\r')
        {
            fbDevice.cursor.xPosition = 0;
        }
        else if (*s == '\n')
        {
            fbDevice.cursor.xPosition = 0;
            fbDevice.cursor.yPosition += FONT_HEIGHT;
        }
        else
        {
            drawChar(*s, fbDevice.cursor.xPosition, fbDevice.cursor.yPosition, r, g, b, overlay);
            fbDevice.cursor.xPosition += FONT_WIDTH;
        }

        s++;
    }
}

void fbWrite(const char *str)
{
    if (*fbDevice.owner != GetCurrentPID())
        return;

    drawString(str, colors[0], colors[1], colors[2], false);
    SetIODeviceCursor(fbDevice.category, fbDevice.code, fbDevice.cursor);
}

bool fbRequest(unsigned int code, void *data)
{
    if (*fbDevice.owner != GetCurrentPID())
        return false;

    switch (code)
    {
    case FB_SET_COLOR_REQUEST:
        colors[0] = ((unsigned char *)data)[0];
        colors[1] = ((unsigned char *)data)[1];
        colors[2] = ((unsigned char *)data)[2];
        return true;

    default:
        return false;
    }
}

bool FramebufferInit()
{
    framebuffer = 0;

    mailbox[0] = 35 * 4;
    mailbox[1] = MAILBOX_REQUEST;

    mailbox[2] = SET_PHYSICAL_WIDTH_HEIGHT;
    mailbox[3] = 8;
    mailbox[4] = 0;
    mailbox[5] = 1920;
    mailbox[6] = 1080;

    mailbox[7] = SET_VIRTUAL_WIDTH_HEIGHT;
    mailbox[8] = 8;
    mailbox[9] = 8;
    mailbox[10] = 1920;
    mailbox[11] = 1080;

    mailbox[12] = SET_VIRTUAL_OFFSET;
    mailbox[13] = 8;
    mailbox[14] = 8;
    mailbox[15] = 0;
    mailbox[16] = 0;

    mailbox[17] = SET_DEPTH;
    mailbox[18] = 4;
    mailbox[19] = 4;
    mailbox[20] = 32;

    mailbox[21] = SET_PIXEL_ORDER;
    mailbox[22] = 4;
    mailbox[23] = 4;
    mailbox[24] = 1; // RGB

    mailbox[25] = ALLOCATE_BUFFER;
    mailbox[26] = 8;
    mailbox[27] = 8;
    mailbox[28] = 4096;
    mailbox[29] = 0;

    mailbox[30] = GET_PITCH;
    mailbox[31] = 4;
    mailbox[32] = 4;
    mailbox[33] = 0;

    mailbox[34] = 0;

    if (CallMailbox(GPU_TO_ARM) && mailbox[20] == 32 && mailbox[28] != 0)
    {
        mailbox[28] &= 0x3FFFFFFF;
        width = mailbox[10];
        height = mailbox[11];
        pitch = mailbox[33];
        isrgb = mailbox[24];
        framebuffer = (unsigned char *)PHYS_TO_VIRT(((unsigned long)mailbox[28]));

        fbDevice.category = IO_FRAMEBUFFER;
        fbDevice.read = 0;
        fbDevice.write = fbWrite;
        fbDevice.request = fbRequest;
        fbDevice.notify = 0;
        fbDevice.code = 0;
        fbDevice.owner = malloc(sizeof(PID));

        colors[0] = 0xFF;
        colors[1] = 0xFF;
        colors[2] = 0xFF;

        AddIODevice(fbDevice);

        LOG("Framebuffer initialized.\n");
        return true;
    }

    LOG("Framebuffer failed initialization (%lu).\n", framebuffer);
    LOG("Values: %u %u %u %u\n", mailbox[0], mailbox[1], mailbox[20], mailbox[28]);
    return false;
}