/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lib/string.h>
#include <memory/memory.h>

unsigned long strlen(const char *str)
{
    unsigned long length = 0;
    while (str[length] != '\0')
        length++;

    return length;
}

unsigned long ustrlen(const unsigned char *str)
{
    unsigned long length = 0;
    while (str[length] != '\0')
        length++;

    return length;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    return (*s1 > *s2) - (*s2 > *s1);
}

void strcpy(void *dest, const void *source)
{
    int i = 0;
    while ((((unsigned char *)dest)[i] = ((unsigned char *)source)[i]) != '\0')
        i++;
}

char *strrchr(const char *s, int c)
{
    const char *p = 0;

    for (;;)
    {
        if (*s == (char)c)
            p = s;
        if (*s++ == '\0')
            return (char *)p;
    }
}

char *strcat(char *dest, const char *src)
{
    char *rdest = dest;

    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return rdest;
}

int ustrcmp(const unsigned char *s1, const unsigned char *s2)
{
    while (*s1 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    return (*s1 > *s2) - (*s2 > *s1);
}

char toupper(char x)
{
    if (x >= 'a' && x <= 'z')
        return x - ('a' - 'A');
    return x;
}

unsigned char utoupper(unsigned char x)
{
    if (x >= 'a' && x <= 'z')
        return x - ('a' - 'A');
    return x;
}

void memset(void *dest, int value, unsigned long size)
{
    unsigned char *ptr = (unsigned char *)dest;
    while (size--)
        *ptr++ = (unsigned char)value;

    return;
}

void memcpy(void *dst, const void *src, unsigned long size)
{
    for (int i = 0; i < size; i++)
        ((char *)dst)[i] = ((char *)src)[i];
}

int memcmp(const void *s1, const void *s2, unsigned long n)
{
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;

    for (unsigned long i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
            return p1[i] - p2[i];
    }

    return 0;
}