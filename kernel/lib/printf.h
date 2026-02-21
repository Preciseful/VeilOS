#pragma once
#pragma GCC diagnostic ignored "-Wmultistatement-macros"

#include <stdarg.h>

#define PRINTF_MAGIC_PADDING "                     "
#define LOG(x, ...)                                                                             \
    Printf("[%s]%s" x, __func__, PRINTF_MAGIC_PADDING + (sizeof(__func__) - 1), ##__VA_ARGS__); \
    _Static_assert(sizeof(__func__) - 1 <= sizeof(PRINTF_MAGIC_PADDING), "Function name too long for printing.");

void SetPrintf(void (*p)(char c));
unsigned int SPrintf(char *dst, char *fmt, ...);
void Printf(char *fmt, ...);