#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

enum FRAMEBUFFER_TAGS
{
    ALLOCATE_BUFFER = 0x40001,
    RELEASE_BUFFER = 0x48001,

    GET_PHYSICAL_WIDTH_HEIGHT = 0x40003,
    SET_PHYSICAL_WIDTH_HEIGHT = 0x48003,

    GET_VIRTUAL_WIDTH_HEIGHT = 0x40004,
    SET_VIRTUAL_WIDTH_HEIGHT = 0x48004,

    GET_DEPTH = 0x40005,
    SET_DEPTH = 0x48005,

    GET_PIXEL_ORDER = 0x40006,
    SET_PIXEL_ORDER = 0x48006,

    GET_ALPHA_MODE = 0x40007,
    SET_ALPHA_MODE = 0x48007,

    GET_PITCH = 0x40008,

    GET_VIRTUAL_OFFSET = 0x40009,
    SET_VIRTUAL_OFFSET = 0x48009,

    GET_OVERSCAN = 0x4000a,
    SET_OVERSCAN = 0x4800a,

    GET_PALETTE = 0x4000b,
    SET_PALETTE = 0x4800b,

    SET_CURSOR_INFO = 0x8010,
    SET_CURSOR_STATE = 0x8011,
};

extern unsigned char *framebuffer;

void framebuffer_init();
void drawChar(unsigned char ch, int x, int y, unsigned char attr, int zoom);
void drawString(int x, int y, char *s, unsigned char attr, int zoom);
void framebuffer_putc(char c, unsigned char attr);

#endif