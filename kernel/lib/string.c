#include <lib/string.h>

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