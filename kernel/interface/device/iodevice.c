#include <interface/device/iodevice.h>
#include <lib/string.h>
#include <scheduler/scheduler.h>

typedef struct IOLink
{
    struct IOLink *child;
    struct IOLink *next;
    enum IO_Category category;
    IODevice device;
} IOLink;

IOLink *start_link = 0;

IODevice *getInternalDevice(enum IO_Category category, unsigned int value)
{
    PID pid = GetCurrentPID();
    IOLink *link = start_link;

    while (link)
    {
        if (link->category != category)
        {
            link = link->next;
            continue;
        }

        IOLink *child = link->child;
        while (child)
        {
            if (child->device.code == value)
                break;
            child = child->next;
        }

        if (child == 0)
            return 0;

        if (*child->device.owner == -1)
            *child->device.owner = pid;

        if (*child->device.owner != pid)
            return false;

        return &child->device;
    }

    return 0;
}

bool GetIODevice(enum IO_Category category, unsigned int value, IODevice *device)
{
    IODevice *intdevice = getInternalDevice(category, value);
    if (intdevice == 0)
        return false;

    memcpy(device, intdevice, sizeof(IODevice));
    return true;
}

IOLink *createLink(enum IO_Category category)
{
    IOLink *link = (IOLink *)malloc(sizeof(IOLink));
    link->category = category;
    link->child = 0;
    link->next = 0;

    return link;
}

void AddIODevice(IODevice device)
{
    *device.owner = -1;

    if (start_link == 0)
        start_link = createLink(device.category);

    IOLink *link = start_link;
    while (link)
    {
        if (link->category == device.category)
            break;
        link = link->next;
    }

    if (link == 0)
    {
        link = createLink(device.category);
        link->next = start_link->next;
        start_link->next = link;
    }

    IOLink *child = createLink(device.category);
    child->next = link->child;
    child->device = device;
    child->child = 0;

    link->child = child;
}

void SetIODeviceNotify(enum IO_Category category, unsigned int value, void (*notify)(unsigned int code, void *data))
{
    IODevice *device = getInternalDevice(category, value);
    if (device == 0)
        return;

    if (GetCurrentPID() == *device->owner)
        device->notify = notify;
}

void SetIODeviceCursor(enum IO_Category category, unsigned int value, IODeviceCursor cursor)
{
    IODevice *device = getInternalDevice(category, value);
    if (device == 0)
        return;

    device->cursor = cursor;
}