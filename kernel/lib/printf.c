#include "printf.h"
#include <stdbool.h>

static void (*printf_putc)(char c);

void SetPrintf(void (*p)(char c))
{
    printf_putc = p;
}

static inline char *copystring(char *dst, const char *buf)
{
    while (*buf)
    {
        if (dst == (void *)0)
            printf_putc(*buf++);
        else
            *dst++ = *buf++;
    }

    return dst;
}

static char *handlesigned(long sarg, char *dst, unsigned long padlen, char pad)
{
    unsigned long arg;
    char buf[21];
    bool negative = false;

    if (sarg < 0)
    {
        negative = true;
        arg = -(unsigned long)sarg;
    }
    else
        arg = (unsigned long)sarg;

    int i = 20;
    buf[i] = '\0';

    do
    {
        buf[--i] = '0' + (arg % 10);
        arg /= 10;
    } while (arg != 0 && i > 0);

    int digits = 20 - i;
    int padding = 0;
    if (padlen > (unsigned long)digits)
        padding = padlen - digits;

    if (negative && padding > 0)
        padding--;

    while (padding-- > 0 && i > 0)
        buf[--i] = pad;

    if (negative && i > 0)
        buf[--i] = '-';

    return copystring(dst, &buf[i]);
}

static char *handleunsigned(unsigned long arg, char *dst, unsigned long padlen, char pad)
{
    char buf[21];
    int i = 20;
    buf[i] = '\0';

    do
    {
        buf[--i] = '0' + (arg % 10);
        arg /= 10;
    } while (arg != 0 && i > 0);

    int digits = 20 - i;
    int padding = 0;
    if (padlen > (unsigned long)digits)
        padding = padlen - digits;

    while (padding-- > 0 && i > 0)
        buf[--i] = pad;

    return copystring(dst, &buf[i]);
}

static char *handlehexa(unsigned long arg, char *dst, unsigned long padlen, char pad, bool uppercase)
{
    char hexastart = uppercase ? 'A' - 10 : 'a' - 10;

    char buf[21];
    int i = 20;
    buf[i] = '\0';

    do
    {
        char n = arg & 0x0f;
        buf[--i] = n + (n > 9 ? hexastart : '0');
        arg >>= 4;
    } while (arg != 0 && i > 0);

    int digits = 20 - i;
    int padding = 0;
    if (padlen > (unsigned long)digits)
        padding = padlen - digits;

    while (padding-- > 0)
        buf[--i] = pad;

    return copystring(dst, &buf[i]);
}

static unsigned int vsprintf(char *dst, char *fmt, va_list args)
{
    char *orig = dst;
    unsigned long len;
    char pad = ' ';
    bool long_specifier;

    while (*fmt)
    {
        if (*fmt != '%')
        {
            if (dst == 0)
                printf_putc(*fmt);
            else
                *dst++ = *fmt;
            fmt++;
            continue;
        }

        len = 0;
        pad = ' ';
        fmt++;
        long_specifier = false;

        if (*fmt == '%')
        {
            if (dst == 0)
                printf_putc(*fmt);
            else
                *dst++ = *fmt;
            fmt++;
            continue;
        }

        if (*fmt == '0')
        {
            pad = '0';
            fmt++;
        }

        while (*fmt >= '0' && *fmt <= '9')
        {
            len *= 10;
            len += *fmt - '0';
            fmt++;
        }

        while (*fmt == 'l')
        {
            long_specifier = true;
            fmt++;
        }

        if (*fmt == 'c')
        {
            int arg = va_arg(args, int);
            if (dst == 0)
                printf_putc((char)arg);
            else
                *dst++ = (char)arg;
            fmt++;
            continue;
        }

        else if (*fmt == 'i' || *fmt == 'd')
        {
            if (long_specifier)
            {
                long arg = va_arg(args, long);
                dst = handlesigned(arg, dst, len, pad);
            }
            else
            {
                int arg = va_arg(args, int);
                dst = handlesigned(arg, dst, len, pad);
            }
        }
        else if (*fmt == 'u')
        {
            if (long_specifier)
            {
                unsigned long arg = va_arg(args, unsigned long);
                dst = handleunsigned(arg, dst, len, pad);
            }
            else
            {
                unsigned int arg = va_arg(args, unsigned int);
                dst = handleunsigned(arg, dst, len, pad);
            }
        }
        else if (*fmt == 's')
        {
            char *arg = va_arg(args, char *);
            if (arg == (void *)0)
                dst = copystring(dst, "(null)");
            else
                dst = copystring(dst, arg);
        }

        else if (*fmt == 'x' || *fmt == 'X')
        {
            bool uppercase = (*fmt == 'x') ? false : true;

            if (long_specifier)
            {
                unsigned long arg = va_arg(args, unsigned long);
                dst = handlehexa(arg, dst, len, pad, uppercase);
            }
            else
            {
                unsigned int arg = va_arg(args, unsigned int);
                dst = handlehexa(arg, dst, len, pad, uppercase);
            }
        }

        fmt++;
    }

    if (dst != 0)
        *dst = 0;
    return dst - orig;
}

unsigned int SPrintf(char *dst, char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    unsigned int result = vsprintf(dst, fmt, va);
    va_end(va);
    return result;
}

void Printf(char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsprintf(0, fmt, va);
    va_end(va);
}