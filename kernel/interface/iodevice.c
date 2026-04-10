/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/iodevice.h>
#include <lib/string.h>
#include <scheduler/scheduler.h>
#include <lib/panic.h>
#include <lib/printf.h>
#include <interface/errno.h>

#define CHECK_AVAILABILITY(perm, func)                                                            \
    if (token.permissions & IO_##perm)                                                            \
    {                                                                                             \
        if (!(intdevice->flags & IO_MULTIPLE_##perm) && intdevice->permissions_taken & IO_##perm) \
            return -1;                                                                            \
        if (intdevice->func == 0)                                                                 \
            return -1;                                                                            \
    }

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

TokenID ownIODeviceWithToken(enum IO_Category category, DID code, IODeviceToken token)
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
            intdevice->tokens[i] = token;
            return i;
        }
    }

    intdevice->tokens_length++;
    intdevice->tokens = realloc(intdevice->tokens, sizeof(IODeviceToken) * intdevice->tokens_length);
    intdevice->tokens[intdevice->tokens_length - 1] = token;
    return intdevice->tokens_length - 1;
}

TokenID OwnIODevice(enum IO_Category category, DID code, enum IO_Permissions permission)
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

bool AddIODevice(IODevice device)
{
    device.tokens = 0;
    device.tokens_length = 0;
    device.permissions_taken = 0;

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

    IOLink *neighbour = link->child;
    while (neighbour)
    {
        if (neighbour->device.code == device.code)
            return false;

        neighbour = neighbour->next;
    }

    IOLink *child = createLink(device.category);
    child->next = link->child;
    child->device = device;
    child->child = 0;

    link->child = child;

    return true;
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

void FreeIODevice(TokenID token, enum IO_Category category, DID code)
{
    IODevice *device = getInternalDevice(category, code);
    if (token < 0)
        return;
    if (device == 0)
        return;
    if (token >= device->tokens_length)
        return;
    if (device->tokens[token].available)
        return;
    if (device->tokens[token].process != GetCurrentPID())
        return;

    if (device->read != 0 && !(device->flags & IO_MULTIPLE_READ) && device->tokens[token].permissions & IO_READ)
        device->permissions_taken &= ~IO_READ;
    if (device->write != 0 && !(device->flags & IO_MULTIPLE_WRITE) && device->tokens[token].permissions & IO_WRITE)
        device->permissions_taken &= ~IO_WRITE;
    if (device->request != 0 && !(device->flags & IO_MULTIPLE_REQUEST) && device->tokens[token].permissions & IO_REQUEST)
        device->permissions_taken &= ~IO_REQUEST;

    device->tokens[token].available = true;
}

bool checkDevice(IODevice *device, TokenID token, enum IO_Permissions permission)
{
    if (token < 0)
        return false;
    if (device == 0)
        return false;
    if (token >= device->tokens_length)
        return false;
    if (device->tokens[token].available)
        return false;
    if (device->tokens[token].process != GetCurrentPID())
        return false;
    if (!(device->tokens[token].permissions & permission))
        return false;

    return true;
}

long ReadIODevice(TokenID token, enum IO_Category category, DID code, char *buf, unsigned long len)
{
    IODevice *device = getInternalDevice(category, code);
    if (!checkDevice(device, token, IO_READ))
        return -E_NO_IO_DEVICE;

    return device->read(token, buf, len);
}

long WriteIODevice(TokenID token, enum IO_Category category, DID code, const char *buf, unsigned long len)
{
    IODevice *device = getInternalDevice(category, code);
    if (!checkDevice(device, token, IO_WRITE))
        return -E_NO_IO_DEVICE;

    return device->write(token, buf, len);
}

bool RequestIODevice(TokenID token, enum IO_Category category, DID code, unsigned int requestMessage, void *data)
{
    IODevice *device = getInternalDevice(category, code);
    if (!checkDevice(device, token, IO_REQUEST))
        return false;

    return device->request(token, requestMessage, data);
}

void SetIODeviceCursor(TokenID token, enum IO_Category category, DID code, IODeviceCursor cursor)
{
    IODevice *device = getInternalDevice(category, code);
    if (!checkDevice(device, token, IO_REQUEST))
        return;

    device->cursor = cursor;
}

SYSCALL_HANDLER(own_device)
{
    IODeviceToken token;
    token.permissions = sp->x2;
    token.process = GetCurrentPID();
    token.available = false;

    int result = ownIODeviceWithToken(sp->x0, sp->x1, token);
    if (result == -1)
        return -1;

    IODeviceOwnership ownership;
    ownership.category = sp->x0;
    ownership.code = sp->x1;
    ownership.token = result;
    Task *task = GetRunningTask();

    for (int i = 0; i < task->devices_count; i++)
    {
        if (task->devices[i].token == -1)
        {
            task->devices[i] = ownership;
            return result;
        }
    }

    task->devices_count++;
    task->devices = realloc(task->devices, sizeof(IODeviceOwnership) * task->devices_count);
    task->devices[task->devices_count - 1] = ownership;

    return result;
}

SYSCALL_HANDLER(read_device)
{
    TokenID token = sp->x0;
    enum IO_Category category = sp->x1;
    DID code = sp->x2;

    char *buf = (char *)sp->x3;
    unsigned long len = sp->x4;

    return ReadIODevice(token, category, code, buf, len);
}

SYSCALL_HANDLER(write_device)
{
    TokenID token = sp->x0;
    enum IO_Category category = sp->x1;
    DID code = sp->x2;

    const char *buf = (const char *)sp->x3;
    unsigned long len = sp->x4;

    return WriteIODevice(token, category, code, buf, len);
}

SYSCALL_HANDLER(request_device)
{
    TokenID token = sp->x0;
    enum IO_Category category = sp->x1;
    DID code = sp->x2;

    unsigned int request_message = sp->x3;
    void *data = (void *)sp->x4;

    return RequestIODevice(token, category, code, request_message, data);
}

SYSCALL_HANDLER(free_device)
{
    TokenID token = sp->x0;
    enum IO_Category category = sp->x1;
    DID code = sp->x2;

    FreeIODevice(token, category, code);

    Task *task = GetRunningTask();
    IODeviceOwnership *devices = task->devices;

    for (int i = 0; i < task->devices_count; i++)
    {
        if (devices[i].token == token)
        {
            devices[i].token = -1;
            break;
        }
    }

    return 0;
}