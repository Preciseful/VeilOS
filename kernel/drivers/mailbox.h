#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdbool.h>
#include <boot/base.h>

#define MAILBOX_BASE PERIPHERAL_BASE + 0xB880
#define MAILBOX_READ MAILBOX_BASE + 0x0
#define MAILBOX_POLL MAILBOX_BASE + 0x10
#define MAILBOX_SENDER MAILBOX_BASE + 0x14
#define MAILBOX_STATUS MAILBOX_BASE + 0x18
#define MAILBOX_CONFIG MAILBOX_BASE + 0x1C
#define MAILBOX_WRITE MAILBOX_BASE + 0x20

#define MAILBOX_REQUEST 0x0
#define MAILBOX_RESPONSE 0x80000000
#define MAILBOX_SUCCESS 0x80000000
#define MAILBOX_FAIL 0x80000001
#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

enum Mailbox_Clocks
{
    EMMC_CLOCK = 1
};

enum Mailbox_Channels
{
    POWER_MANAGEMENT_CHANNEL,
    FRAMEBUFFER_CHANNEL,
    VIRTUAL_UART_CHANNEL,
    VCHIQ_CHANNEL,
    LEDS_CHANNEL,
    BUTTONS_CHANNEL,
    TOUCH_SCREEN_CHANNEL,
    ARM_TO_GPU,
    GPU_TO_ARM
};

extern volatile unsigned int mailbox[36];

unsigned int mailbox_clock(enum Mailbox_Clocks clock);
bool mailbox_call(unsigned char channel);

#endif