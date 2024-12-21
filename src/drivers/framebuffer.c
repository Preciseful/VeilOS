#include <drivers/framebuffer.h>
#include <mailbox.h>
#include <lib/font.h>
#include <drivers/miniuart.h>
#include <lib/printf.h>

unsigned char *framebuffer;
unsigned int width, height;
unsigned int pitch;
unsigned int isrgb;
unsigned int xPosition, yPosition;

void framebuffer_init()
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

    if (mailbox_call(GPU_TO_ARM) && mailbox[20] == 32 && mailbox[28] != 0)
    {

        mailbox[28] &= 0x3FFFFFFF;
        width = mailbox[10];
        height = mailbox[11];
        pitch = mailbox[33];
        isrgb = mailbox[24];
        framebuffer = (unsigned char *)((long)mailbox[28]);
    }

    printf_use_framebuffer = true;
}

void drawPixel(int x, int y, unsigned char attr)
{
    int offs = (y * pitch) + (x * 4);
    *((unsigned int *)(framebuffer + offs)) = vgapal[attr & 0x0f];
}

void drawChar(unsigned char ch, int x, int y, unsigned char attr, int zoom)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i = 0; i <= (FONT_HEIGHT * zoom); i++)
    {
        for (int j = 0; j < (FONT_WIDTH * zoom); j++)
        {
            unsigned char mask = 1 << (j / zoom);
            unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

            drawPixel(x + j, y + i, col);
        }
        glyph += (i % zoom) ? 0 : FONT_BPL;
    }
}

void drawString(int x, int y, char *s, unsigned char attr, int zoom)
{
    while (*s)
    {
        if (*s == '\r')
        {
            x = 0;
        }
        else if (*s == '\n')
        {
            x = 0;
            y += (FONT_HEIGHT * zoom);
        }
        else
        {
            drawChar(*s, x, y, attr, zoom);
            x += (FONT_WIDTH * zoom);
        }
        s++;
    }
}
void framebuffer_putc(char c, unsigned char attr)
{
    if (xPosition >= width)
        xPosition = 0;
    if (yPosition >= height)
        yPosition = 0;

    if (c == '\r')
    {
        xPosition = 0;
    }
    else if (c == '\n')
    {
        xPosition = 0;
        yPosition += FONT_HEIGHT * 3 + 3;
    }
    else
    {
        drawChar(c, xPosition, yPosition, attr, 3);
        xPosition += FONT_WIDTH * 3 + 3;
    }
}