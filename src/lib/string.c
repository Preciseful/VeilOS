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

int memcmp(const unsigned char *s1, const unsigned char *s2, unsigned long size)
{
    int i = 0;
    while (i < size && *s1 && *s1 == *s2)
    {
        ++s1;
        ++s2;
        ++i;
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

bool strcontains(const unsigned char *sub, const unsigned char *s)
{
    unsigned long sublen = strlen(sub);
    unsigned long slen = strlen(s);
    if (sublen > slen)
        return false;
    if (sublen == slen)
        return (strcmp(s, sub) == 0);

    for (unsigned long i = 0; i < slen - sublen; i++)
    {
        if (s[i] == sub[0])
        {
            bool ok = true;

            for (unsigned long j = 1; j < sublen; j++)
            {
                if (s[i + j] != sub[j])
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
                return true;
        }
    }

    return false;
}

unsigned char toupper(unsigned char x)
{
    if (x >= 'a' && x <= 'z')
        return x - ('a' - 'A');
    return x;
}

unsigned char *strcat(unsigned char *x, unsigned char *y)
{
    unsigned char *ptr = x;

    while (*ptr)
        ptr++;
    while (*y)
        *ptr++ = *y++;

    *ptr = '\0';
    return x;
}