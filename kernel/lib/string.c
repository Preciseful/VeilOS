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

List GetSubstrings(const char *string, char split)
{
    List list = CreateList(LIST_ARRAY);

    unsigned long len = strlen(string);
    unsigned long start = 0;

    for (unsigned long i = 0; i <= len; i++)
    {
        if (string[i] != split && string[i] != 0)
            continue;

        unsigned long sub_len = i - start;

        char *sub = malloc(sub_len + 1);
        memcpy(sub, string + start, sub_len);
        sub[sub_len] = 0;

        AddToList(&list, sub);

        start = i + 1;
    }

    return list;
}