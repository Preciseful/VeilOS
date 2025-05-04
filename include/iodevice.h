#ifndef IODEVICE_H
#define IODEVICE_H

#include <stdbool.h>

struct io_device
{
    enum
    {
        ACTIVE,
        INACTIVE,
        ZOMBIE,
    } status;

    unsigned long index;
    char buffer[16 * 1024];
    void (*put)(char s);
    char (*read)(void);
};

bool iosend(struct io_device *device, char s);
void iocollect(struct io_device *device, char s);

#endif