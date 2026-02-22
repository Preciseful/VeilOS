#include <interface/iodevice.h>
#include <lib/string.h>
#include <scheduler/scheduler.h>
#include <lib/panic.h>

#define CHECK_AVAILABILITY(perm, func)                                                            \
    if (token.permissions & IO_##perm)                                                            \
    {                                                                                             \
        if (!(intdevice->flags & IO_MULTIPLE_##perm) && intdevice->permissions_taken & IO_##perm) \
            return false;                                                                         \
        if (intdevice->func == 0)                                                                 \
            return false;                                                                         \
    }

#define CHECK_DEVICE(permission)                           \
    if (device == 0)                                       \
        return;                                            \
    if (token >= device->tokens_length)                    \
        return;                                            \
    if (device->tokens[token].available)                   \
        return;                                            \
    if (device->tokens[token].process != GetCurrentPID())  \
        return;                                            \
    if (!(device->tokens[token].permissions & permission)) \
        return;

typedef struct IOLink
{
    struct IOLink *child;
    struct IOLink *next;
    enum IO_Category category;
    IODevice device;
} IOLink;

IOLink *start_link = 0;

IODevice *getInternalDevice(enum IO_Category category, DID code)
{
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
            if (child->device.code == code)
                break;
            child = child->next;
        }

        if (child == 0)
            return 0;

        return &child->device;
    }

    return 0;
}

int ownIODeviceWithToken(enum IO_Category category, DID code, IODeviceToken token)
{
    IODevice *intdevice = getInternalDevice(category, code);
    if (intdevice == 0)
        return -1;

    CHECK_AVAILABILITY(READ, read)
    CHECK_AVAILABILITY(WRITE, write)
    CHECK_AVAILABILITY(REQUEST, request)

    if (!(intdevice->flags & IO_MULTIPLE_READ) && token.permissions & IO_READ)
        intdevice->permissions_taken |= IO_READ;

    if (!(intdevice->flags & IO_MULTIPLE_REQUEST) && token.permissions & IO_REQUEST)
        intdevice->permissions_taken |= IO_REQUEST;

    if (!(intdevice->flags & IO_MULTIPLE_WRITE) && token.permissions & IO_WRITE)
        intdevice->permissions_taken |= IO_WRITE;

    if (intdevice->tokens_length == 0)
    {
        intdevice->tokens = malloc(sizeof(IODeviceToken));
        intdevice->tokens[0] = token;
        intdevice->tokens_length++;

        return 0;
    }

    for (unsigned long i = 0; i < intdevice->tokens_length; i++)
    {
        if (intdevice->tokens[i].available)
        {
            intdevice->tokens[0] = token;
            return i;
        }
    }

    intdevice->tokens_length++;
    intdevice->tokens = realloc(intdevice->tokens, sizeof(IODeviceToken) * intdevice->tokens_length);
    intdevice->tokens[intdevice->tokens_length - 1] = token;
    return intdevice->tokens_length - 1;
}

int OwnIODevice(enum IO_Category category, DID code, enum IO_Permissions permission)
{
    IODeviceToken token;
    token.permissions = permission;
    token.process = GetCurrentPID();
    token.available = false;

    return ownIODeviceWithToken(category, code, token);
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
    device.tokens = 0;
    device.tokens_length = 0;

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

IODeviceToken *getToken(unsigned long pid, IODevice device)
{
    for (unsigned long i = 0; i < device.tokens_length; i++)
    {
        if (device.tokens[i].process == pid)
            return &device.tokens[i];
    }

    return 0;
}

void FreeIODevice(int token, enum IO_Category category, DID code)
{
    IODevice *device = getInternalDevice(category, code);
    if (device == 0)
        return;
    if (token >= device->tokens_length)
        return;
    if (!device->tokens[token].available)
        return;
    if (device->tokens[token].process != GetCurrentPID())
        return;

    device->tokens[token].available = true;
}

void ReadIODevice(int token, enum IO_Category category, DID code, char *buf, unsigned long len)
{
    IODevice *device = getInternalDevice(category, code);
    CHECK_DEVICE(IO_READ)

    device->read(token, buf, len);
}

void WriteIODevice(int token, enum IO_Category category, DID code, const char *buf)
{
    IODevice *device = getInternalDevice(category, code);
    CHECK_DEVICE(IO_WRITE)

    device->write(token, buf);
}

void RequestIODevice(int token, enum IO_Category category, DID code, unsigned int requestMessage, void *data)
{
    IODevice *device = getInternalDevice(category, code);
    CHECK_DEVICE(IO_WRITE)

    device->request(token, requestMessage, data);
}

void SetIODeviceCursor(int token, enum IO_Category category, DID code, IODeviceCursor cursor)
{
    IODevice *device = getInternalDevice(category, code);
    CHECK_DEVICE(IO_REQUEST)

    device->cursor = cursor;
}

SYSCALL_HANDLER(own_device)
{
    IODeviceToken token;
    token.permissions = sp->x2;
    token.process = GetCurrentPID();
    token.available = false;

    return ownIODeviceWithToken(sp->x0, sp->x1, token);
}
