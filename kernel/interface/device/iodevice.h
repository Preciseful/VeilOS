#pragma once

#include <scheduler/task.h>

enum IO_Category
{
    IO_CONSOLE,
    IO_UART,
    IO_FRAMEBUFFER,
};

typedef struct IODeviceCursor
{
    unsigned int xPosition, yPosition;
    bool cursorVisible;
} IODeviceCursor;

typedef struct IODevice
{
    void (*read)(char *buf, unsigned long length);
    void (*write)(const char *buf);
    void (*notify)(unsigned int code, void *data);
    bool (*request)(unsigned int code, void *data);

    IODeviceCursor cursor;

    enum IO_Category category;
    unsigned int code;
    PID *owner;
} IODevice;

bool GetIODevice(enum IO_Category category, unsigned int value, IODevice *device);
void AddIODevice(IODevice device);
void SetIODeviceNotify(enum IO_Category category, unsigned int value, void (*notify)(unsigned int code, void *data));
void SetIODeviceCursor(enum IO_Category category, unsigned int value, IODeviceCursor cursor);