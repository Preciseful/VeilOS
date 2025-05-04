#include <iodevice.h>
#include <lib/string.h>

bool iosend(struct io_device *device, char s)
{
    switch (device->status)
    {
    case ACTIVE:
        device->put(s);
        break;

    case INACTIVE:
        iocollect(device, s);
        break;

    case ZOMBIE:
        return false;
    }

    return true;
}

void iocollect(struct io_device *device, char s)
{
    device->buffer[device->index] = s;
    device->index++;
}