#ifndef DEBUG_H
#define DEBUG_H

#include <lib/printf.h>

void debug_ulvar(const void *x, unsigned long v)
{
    printf("%c%s&%lu\n", 141, x, v);
}

void debug_ldvar(const void *x, long v)
{
    printf("%c%s&%ld\n", 141, x, v);
}

void debug_dvar(const void *x, int v)
{
    printf("%c%s&%d\n", 141, x, vfree);
}

void debug_uvar(const void *x, unsigned int v)
{
    printf("%c%s&%u\n", 141, x, v);
}

#endif