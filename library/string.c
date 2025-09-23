#include <string.h>

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