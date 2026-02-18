#include <drivers/framebuffer.h>
#include <drivers/mailbox.h>
#include <lib/printf.h>
#include <memory/mmu.h>

unsigned char *framebuffer;
unsigned int width, height;
unsigned int pitch;
unsigned int isrgb;

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

        LOG("Framebuffer initialized.\n")
        return true;
    }

    LOG("Framebuffer failed initialization (%lu).\n", framebuffer);
    LOG("Values: %u %u %u %u\n", mailbox[0], mailbox[1], mailbox[20], mailbox[28]);
    return false;
}

void DrawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
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
