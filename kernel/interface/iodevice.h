/**
 * @file
 * @author Developful
 * @brief Interface of IO devices.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <scheduler/task.h>

typedef int TokenID;
typedef unsigned int DID;

enum IO_Category
{
    IO_UART,
    IO_FRAMEBUFFER,
    IO_CONSOLE,
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

typedef struct IODeviceOwnership
{
    TokenID token;
    enum IO_Category category;
    DID code;
} IODeviceOwnership;

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
    long (*read)(TokenID token, char *buf, unsigned long length);
    long (*write)(TokenID token, const char *buf, unsigned long length);
    bool (*request)(TokenID token, unsigned int code, void *data);

    IODeviceCursor cursor;
    IODeviceToken *tokens;
    int tokens_length;

    enum IO_Permissions permissions_taken;
    enum IO_Flags flags;
    enum IO_Category category;
    DID code;
} IODevice;

/**
 * @brief Registers an IO device. The only fields that can be filled are: `read`, `write`, `request`, `category`, `cursor`, `flags`, `code`.
 *
 * @param device The device to be registered.
 * @return `true` if no conflicting devices were found, otherwise `false`.
 */
bool AddIODevice(IODevice device);

/**
 * @brief Own an IO device in order to interact with it.
 *
 * @param category The category of the IO device.
 * @param code The code of the IO device.
 * @param permission The permissions requested.
 * @return A token for further interactions.
 */
TokenID OwnIODevice(enum IO_Category category, DID code, enum IO_Permissions permission);

/**
 * @brief Frees up an IO device, removing ownership of it.
 *
 * @param token The token.
 * @param category The category of the device.
 * @param code The code of the device.
 */
void FreeIODevice(TokenID token, enum IO_Category category, DID code);

/**
 * @brief Sets the cursor of an IODevice.
 * Request permissions are required.
 * @param token The token.
 * @param category The category of the device.
 * @param code The code of the device.
 * @param cursor The cursor state to set.
 */
void SetIODeviceCursor(TokenID token, enum IO_Category category, DID code, IODeviceCursor cursor);

/**
 * @brief Reads data from an IO device.
 *
 * @param token The token.
 * @param category The category of the device.
 * @param code The code of the device.
 * @param buf The buffer to read in.
 * @param len The size of the buffer.
 * @return The amount read, or possible errors.
 */
long ReadIODevice(TokenID token, enum IO_Category category, DID code, char *buf, unsigned long len);

/**
 * @brief Writes data to an IO device.
 *
 * @param token The token.
 * @param category The category of the device.
 * @param code The code of the device.
 * @param buf The buffer to write.
 * @param len The size of the buffer.
 * @return The amount written, or possible errors.
 */
long WriteIODevice(TokenID token, enum IO_Category category, DID code, const char *buf, unsigned long len);
bool RequestIODevice(TokenID token, enum IO_Category category, DID code, unsigned int requestMessage, void *data);

SYSCALL_HANDLER(own_device);
SYSCALL_HANDLER(read_device);
SYSCALL_HANDLER(write_device);
SYSCALL_HANDLER(request_device);
SYSCALL_HANDLER(free_device);