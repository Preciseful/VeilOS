#include <system/debug/semihost.h>
#include <memory/allocator.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <stdarg.h>

typedef struct StdOpenFileParameter
{
    char *path;
    unsigned long mode;
    unsigned long len;
} StdOpenFileParameter;

typedef struct StdSeekFileParameter
{
    unsigned long handle;
    unsigned long seek;
} StdSeekFileParameter;

typedef struct StdWriteFileParameter
{
    unsigned long handle;
    char *buf;
    unsigned long len;
} StdWriteFileParameter;

DFILEHANDLE DebugFileOpen(char *path, unsigned long mode)
{
    StdOpenFileParameter arg;
    arg.path = path;
    arg.len = strlen(path);
    arg.mode = mode;

    return tell_debug_host(Debug_Open, &arg);
}

void DebugFileSeek(DFILEHANDLE handle, unsigned long seek)
{
    StdSeekFileParameter arg;
    arg.handle = handle;
    arg.seek = seek;

    tell_debug_host(Debug_Seek, &arg);
}

unsigned long DebugFileWrite(DFILEHANDLE handle, char *data)
{
    StdWriteFileParameter arg;
    arg.handle = handle;
    arg.buf = data;
    arg.len = strlen(data);

    return tell_debug_host(Debug_Write, &arg);
}

unsigned long DebugFileRead(DFILEHANDLE handle, char *buf, unsigned long len)
{
    // the structs use the same layout
    StdWriteFileParameter arg;
    arg.handle = handle;
    arg.buf = buf;
    arg.len = len;

    return tell_debug_host(Debug_Read, &arg);
}

void DebugFileClose(DFILEHANDLE handle)
{
    tell_debug_host(Debug_Close, &handle);
}

void DebugPrintf(char *fmt, ...)
{
    char result[PAGE_SIZE] = "";

    va_list va;
    va_start(va, fmt);
    SPrintfList(result, fmt, va);
    va_end(va);

    DebugFileWrite(1, result);
}