#include <system/debug/dcc.h>
#include <memory/allocator.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <stdarg.h>

typedef struct StdWriteParameter
{
    unsigned long handle;
    char *buf;
    unsigned long len;
} StdWriteParameter;

void DebugWrite(char *fmt, ...)
{
    char *result = malloc(PAGE_SIZE);

    va_list va;
    va_start(va, fmt);
    SPrintfList(result, fmt, va);
    va_end(va);

    StdWriteParameter args;
    args.handle = 1;
    args.buf = result;
    args.len = strlen(result);

    tell_host(0x05, &args);
    free(result);
}