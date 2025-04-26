#include <lib/string.h>

unsigned long strlen(const unsigned char *str)
{
    unsigned long length = 0;
    while (str[length] != '\0')
        length++;

    return length;
}

int strcmp(const unsigned char *s1, const unsigned char *s2)
{
    while (*s1 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    return (*s1 > *s2) - (*s2 > *s1);
}

int isspace(const unsigned char s)
{
    if (s == ' ')
        return 1;
    return 0;
}

unsigned char *ltrim(const unsigned char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

unsigned char *rtrim(const unsigned char *s)
{
    unsigned char *back = s + strlen(s);
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

unsigned char *trim(const unsigned char *s)
{
    return rtrim(ltrim(s));
}

void *memset(void *dest, int value, unsigned long size)
{
    unsigned char *ptr = (unsigned char *)dest;
    while (size--)
    {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}