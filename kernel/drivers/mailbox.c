#include <drivers/mailbox.h>
#include <boot/base.h>
#include <drivers/gpio.h>

// must be 16 byte aligned
// check mailbox interface
volatile unsigned int __attribute__((aligned(16))) mailbox[36]; // -> 36 is a safe default
                                                                // free to change

unsigned int mailbox_clock(enum Mailbox_Clocks clock)
{
    mailbox[0] = 0;
    mailbox[1] = MAILBOX_REQUEST;
    mailbox[2] = 0x00030002;
    mailbox[3] = 4;
    mailbox[4] = 8;
    mailbox[5] = clock;
    mailbox[6] = 0;
    mailbox[7] = 0;

    mailbox_call(GPU_TO_ARM);
    return mailbox[6];
}

bool mailbox_call(unsigned char channel)
{
    unsigned int request = ((unsigned int)((long)&mailbox) & ~0xF) | (channel & 0xF);

    // wait until its empty
    while (mmio_read(MAILBOX_STATUS) & MAILBOX_FULL)
        ;

    mmio_write(MAILBOX_WRITE, request);

    while (1)
    {
        while (mmio_read(MAILBOX_STATUS) & MAILBOX_EMPTY)
            ;

        if (request == mmio_read(MAILBOX_READ))
            return mailbox[1] == MAILBOX_RESPONSE;
    }

    return false;
}