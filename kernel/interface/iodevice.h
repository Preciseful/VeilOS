#pragma once

#include <scheduler/task.h>

typedef unsigned int DID;

enum IO_Category
{
    IO_CONSOLE,
    IO_UART,
    IO_FRAMEBUFFER,
};

enum IO_Flags
{
    IO_MULTIPLE_READ = 1 << 1,
    IO_MULTIPLE_WRITE = 1 << 2,
    IO_MULTIPLE_REQUEST = 1 << 3,
};

enum IO_Permissions
{
    IO_READ = 1 << 0,
    IO_WRITE = 1 << 1,
    IO_REQUEST = 1 << 2,
};

typedef struct IODeviceToken
{
    enum IO_Permissions permissions;
    PID process;
    bool available;
} IODeviceToken;

typedef struct IODeviceCursor
{
    unsigned int xPosition, yPosition;
    bool cursorVisible;
} IODeviceCursor;

typedef struct IODevice
{
    void (*read)(unsigned int token, char *buf, unsigned long length);
    void (*write)(unsigned int token, const char *buf);
    bool (*request)(unsigned int token, unsigned int code, void *data);

    IODeviceCursor cursor;
    IODeviceToken *tokens;
    int tokens_length;

    enum IO_Permissions permissions_taken;
    enum IO_Flags flags;
    enum IO_Category category;
    DID code;
} IODevice;

int OwnIODevice(enum IO_Category category, DID code, enum IO_Permissions permission);
void AddIODevice(IODevice device);
void FreeIODevice(int token, enum IO_Category category, DID code);
void SetIODeviceCursor(int token, enum IO_Category category, DID code, IODeviceCursor cursor);
void ReadIODevice(int token, enum IO_Category category, DID code, char *buf, unsigned long len);
void WriteIODevice(int token, enum IO_Category category, DID code, const char *buf);

SYSCALL_HANDLER(own_device);